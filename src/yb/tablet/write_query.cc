// Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//

#include "yb/tablet/write_query.h"

#include "yb/client/client.h"
#include "yb/client/error.h"
#include "yb/client/meta_data_cache.h"
#include "yb/client/session.h"
#include "yb/client/table.h"
#include "yb/client/transaction.h"
#include "yb/client/yb_op.h"

#include "yb/common/index.h"
#include "yb/common/row_mark.h"
#include "yb/common/schema.h"

#include "yb/docdb/conflict_resolution.h"
#include "yb/docdb/cql_operation.h"
#include "yb/docdb/doc_write_batch.h"
#include "yb/docdb/pgsql_operation.h"
#include "yb/docdb/redis_operation.h"

#include "yb/tablet/tablet_metadata.h"
#include "yb/tablet/operations/write_operation.h"
#include "yb/tablet/tablet.h"
#include "yb/tablet/tablet_metrics.h"
#include "yb/tablet/transaction_participant.h"
#include "yb/tablet/write_query_context.h"

#include "yb/tserver/tserver.pb.h"

#include "yb/util/logging.h"
#include "yb/util/metrics.h"
#include "yb/util/sync_point.h"
#include "yb/util/trace.h"

using namespace std::placeholders;

namespace yb {
namespace tablet {

namespace {

// Separate Redis / QL / row operations write batches from write_request in preparation for the
// write transaction. Leave just the tablet id behind. Return Redis / QL / row operations, etc.
// in batch_request.
void SetupKeyValueBatch(const tserver::WriteRequestPB& client_request, WritePB* out_request) {
  out_request->set_unused_tablet_id(""); // Backward compatibility.
  auto& out_write_batch = *out_request->mutable_write_batch();
  if (client_request.has_write_batch()) {
    out_write_batch = client_request.write_batch();
  }
  out_write_batch.set_deprecated_may_have_metadata(true);
  if (client_request.has_request_id()) {
    out_request->set_client_id1(client_request.client_id1());
    out_request->set_client_id2(client_request.client_id2());
    out_request->set_request_id(client_request.request_id());
    out_request->set_min_running_request_id(client_request.min_running_request_id());
  }
  out_request->set_batch_idx(client_request.batch_idx());
  // Actually, in production code, we could check for external hybrid time only when there are
  // no ql, pgsql, redis operations.
  // But in CDCServiceTest we have ql write batch with external time.
  if (client_request.has_external_hybrid_time()) {
    out_request->set_external_hybrid_time(client_request.external_hybrid_time());
  }
}

} // namespace

enum class WriteQuery::ExecuteMode {
  kSimple,
  kRedis,
  kCql,
  kPgsql,
};

WriteQuery::WriteQuery(
    int64_t term,
    CoarseTimePoint deadline,
    WriteQueryContext* context,
    Tablet* tablet,
    tserver::WriteResponsePB* response,
    docdb::OperationKind kind)
    : operation_(std::make_unique<WriteOperation>(tablet)),
      term_(term), deadline_(deadline),
      context_(context),
      response_(response),
      kind_(kind),
      start_time_(CoarseMonoClock::Now()) {
}

WritePB& WriteQuery::request() {
  return *operation_->mutable_request();
}

std::unique_ptr<WriteOperation> WriteQuery::PrepareSubmit() {
  operation_->set_completion_callback(
      [operation = operation_.get(), query = this](const Status& status) {
    std::unique_ptr<WriteQuery> query_holder(query);
    query->Finished(operation, status);
  });
  return std::move(operation_);
}

void WriteQuery::DoStartSynchronization(const Status& status) {
  std::unique_ptr<WriteQuery> self(this);
  // Move submit_token_ so it is released after this function.
  ScopedRWOperation submit_token(std::move(submit_token_));
  // If a restart read is required, then we return this fact to caller and don't perform the write
  // operation.
  if (status.ok() && restart_read_ht_.is_valid()) {
    auto restart_time = response()->mutable_restart_read_time();
    restart_time->set_read_ht(restart_read_ht_.ToUint64());
    auto local_limit = context_->ReportReadRestart();
    if (!local_limit.ok()) {
      Cancel(local_limit.status());
      return;
    }
    restart_time->set_deprecated_max_of_read_time_and_local_limit_ht(local_limit->ToUint64());
    restart_time->set_local_limit_ht(local_limit->ToUint64());
    // Global limit is ignored by caller, so we don't set it.
    Cancel(Status::OK());
    return;
  }

  if (!status.ok()) {
    Cancel(status);
    return;
  }

  context_->Submit(self.release()->PrepareSubmit(), term_);
}

void WriteQuery::Release() {
  // Free DocDB multi-level locks.
  docdb_locks_.Reset();
}

WriteQuery::~WriteQuery() {
}

void WriteQuery::set_client_request(std::reference_wrapper<const tserver::WriteRequestPB> req) {
  client_request_ = &req.get();
  read_time_ = ReadHybridTime::FromReadTimePB(req.get());
  allow_immediate_read_restart_ = !read_time_;
}

void WriteQuery::set_client_request(std::unique_ptr<tserver::WriteRequestPB> req) {
  set_client_request(*req);
  client_request_holder_ = std::move(req);
}

void WriteQuery::Finished(WriteOperation* operation, const Status& status) {
  LOG_IF(DFATAL, operation_) << "Finished not submitted operation: " << status;

  if (status.ok()) {
    TabletMetrics* metrics = operation->tablet()->metrics();
    if (metrics) {
      auto op_duration_usec = MonoDelta(CoarseMonoClock::now() - start_time_).ToMicroseconds();
      metrics->write_op_duration_client_propagated_consistency->Increment(op_duration_usec);
    }
  }

  Complete(status);
}

void WriteQuery::Cancel(const Status& status) {
  LOG_IF(DFATAL, !operation_) << "Cancelled submitted operation: " << status;

  Complete(status);
}

void WriteQuery::Complete(const Status& status) {
  Release();

  if (callback_) {
    callback_(status);
  }
}

void WriteQuery::ExecuteDone(const Status& status) {
  scoped_read_operation_.Reset();
  switch (execute_mode_) {
    case ExecuteMode::kSimple:
      SimpleExecuteDone(status);
      return;
    case ExecuteMode::kRedis:
      RedisExecuteDone(status);
      return;
    case ExecuteMode::kCql:
      CqlExecuteDone(status);
      return;
    case ExecuteMode::kPgsql:
      PgsqlExecuteDone(status);
      return;
  }
  FATAL_INVALID_ENUM_VALUE(ExecuteMode, execute_mode_);
}

Result<bool> WriteQuery::PrepareExecute() {
  if (client_request_) {
    auto* request = operation().AllocateRequest();
    SetupKeyValueBatch(*client_request_, request);

    if (!client_request_->redis_write_batch().empty()) {
      return RedisPrepareExecute();
    }

    if (!client_request_->ql_write_batch().empty()) {
      return CqlPrepareExecute();
    }

    if (!client_request_->pgsql_write_batch().empty()) {
      return PgsqlPrepareExecute();
    }

    if (client_request_->has_write_batch() && client_request_->has_external_hybrid_time()) {
      return false;
    }
  } else {
    const auto* request = operation().request();
    if (request && request->has_write_batch() && !request->write_batch().read_pairs().empty()) {
      return SimplePrepareExecute();
    }
  }

  // Empty write should not happen, but we could handle it.
  // Just report it as error in release mode.
  LOG(DFATAL) << "Empty write: " << AsString(client_request_) << ", " << AsString(request());

  return STATUS(InvalidArgument, "Empty write");
}

CHECKED_STATUS WriteQuery::InitExecute(ExecuteMode mode) {
  scoped_read_operation_ = tablet().CreateNonAbortableScopedRWOperation();
  if (!scoped_read_operation_.ok()) {
    return MoveStatus(scoped_read_operation_);
  }
  execute_mode_ = mode;
  return Status::OK();
}

Result<bool> WriteQuery::RedisPrepareExecute() {
  RETURN_NOT_OK(InitExecute(ExecuteMode::kRedis));

  // Since we take exclusive locks, it's okay to use Now as the read TS for writes.
  const auto& redis_write_batch = client_request_->redis_write_batch();

  doc_ops_.reserve(redis_write_batch.size());
  for (const auto& redis_request : redis_write_batch) {
    doc_ops_.emplace_back(new docdb::RedisWriteOperation(redis_request));
  }

  return true;
}

Result<bool> WriteQuery::SimplePrepareExecute() {
  RETURN_NOT_OK(InitExecute(ExecuteMode::kSimple));
  return true;
}

Result<bool> WriteQuery::CqlPrepareExecute() {
  RETURN_NOT_OK(InitExecute(ExecuteMode::kCql));

  auto& metadata = *tablet().metadata();
  DVLOG(2) << "Schema version for  " << metadata.table_name() << ": " << metadata.schema_version();

  const auto& ql_write_batch = client_request_->ql_write_batch();

  doc_ops_.reserve(ql_write_batch.size());

  auto txn_op_ctx = VERIFY_RESULT(tablet().CreateTransactionOperationContext(
      request().write_batch().transaction(),
      /* is_ysql_catalog_table */ false,
      &request().write_batch().subtransaction()));
  auto table_info = metadata.primary_table_info();
  for (const auto& req : ql_write_batch) {
    QLResponsePB* resp = response_->add_ql_response_batch();
    if (!IsSchemaVersionCompatible(
            table_info->schema_version, req.schema_version(),
            req.is_compatible_with_previous_version())) {
      DVLOG(1) << " On " << table_info->table_name
               << " Setting status for write as YQL_STATUS_SCHEMA_VERSION_MISMATCH tserver's: "
               << table_info->schema_version << " vs req's : " << req.schema_version()
               << " is req compatible with prev version: "
               << req.is_compatible_with_previous_version() << " for " << AsString(req);
      resp->set_status(QLResponsePB::YQL_STATUS_SCHEMA_VERSION_MISMATCH);
      resp->set_error_message(Format(
          "schema version mismatch for table $0: expected $1, got $2 (compt with prev: $3)",
          table_info->table_id,
          table_info->schema_version, req.schema_version(),
          req.is_compatible_with_previous_version()));
    } else {
      DVLOG(3) << "Version matches : " << table_info->schema_version << " for "
               << AsString(req);
      auto write_op = std::make_unique<docdb::QLWriteOperation>(
          req, std::shared_ptr<Schema>(table_info, table_info->schema.get()),
          *table_info->index_map, tablet().unique_index_key_schema(),
          txn_op_ctx);
      RETURN_NOT_OK(write_op->Init(resp));
      doc_ops_.emplace_back(std::move(write_op));
    }
  }

  // All operations has wrong schema version
  if (doc_ops_.empty()) {
    return false;
  }

  return true;
}

Result<bool> WriteQuery::PgsqlPrepareExecute() {
  RETURN_NOT_OK(InitExecute(ExecuteMode::kPgsql));

  const auto& pgsql_write_batch = client_request_->pgsql_write_batch();

  doc_ops_.reserve(pgsql_write_batch.size());

  TransactionOperationContext txn_op_ctx;

  auto& metadata = *tablet().metadata();
  bool colocated = metadata.colocated();

  for (const auto& req : pgsql_write_batch) {
    PgsqlResponsePB* resp = response_->add_pgsql_response_batch();
    // Table-level tombstones should not be requested for non-colocated tables.
    if ((req.stmt_type() == PgsqlWriteRequestPB::PGSQL_TRUNCATE_COLOCATED) && !colocated) {
      LOG(WARNING) << "cannot create table-level tombstone for a non-colocated table";
      resp->set_skipped(true);
      continue;
    }
    const std::shared_ptr<tablet::TableInfo> table_info =
        VERIFY_RESULT(metadata.GetTableInfo(req.table_id()));
    if (table_info->schema_version != req.schema_version()) {
      resp->set_status(PgsqlResponsePB::PGSQL_STATUS_SCHEMA_VERSION_MISMATCH);
      resp->set_error_message(
          Format("schema version mismatch for table $0: expected $1, got $2",
                 table_info->table_id,
                 table_info->schema_version,
                 req.schema_version()));
    } else {
      if (doc_ops_.empty()) {
        // Use the value of is_ysql_catalog_table from the first operation in the batch.
        txn_op_ctx = VERIFY_RESULT(tablet().CreateTransactionOperationContext(
            request().write_batch().transaction(),
            table_info->schema->table_properties().is_ysql_catalog_table(),
            &request().write_batch().subtransaction()));
      }
      auto write_op = std::make_unique<docdb::PgsqlWriteOperation>(
          req, *table_info->schema, txn_op_ctx);
      RETURN_NOT_OK(write_op->Init(resp));
      doc_ops_.emplace_back(std::move(write_op));
    }
  }

  // All operations have wrong schema version.
  if (doc_ops_.empty()) {
    return false;
  }

  return true;
}

void WriteQuery::Execute(std::unique_ptr<WriteQuery> query) {
  auto prepare_result = query->PrepareExecute();
  if (!prepare_result.ok()) {
    StartSynchronization(std::move(query), prepare_result.status());
    return;
  }

  if (!prepare_result.get()) {
    StartSynchronization(std::move(query), Status::OK());
    return;
  }

  auto* query_ptr = query.get();
  query_ptr->self_ = std::move(query);
  auto status = query_ptr->DoExecute();
  if (!status.ok()) {
    query_ptr->ExecuteDone(status);
  }
}

CHECKED_STATUS WriteQuery::DoExecute() {
  auto& write_batch = *request().mutable_write_batch();
  isolation_level_ = VERIFY_RESULT(tablet().GetIsolationLevelFromPB(write_batch));
  const RowMarkType row_mark_type = GetRowMarkTypeFromPB(write_batch);
  const auto& metadata = *tablet().metadata();

  const bool transactional_table = metadata.schema()->table_properties().is_transactional() ||
                                   force_txn_path_;

  if (!transactional_table && isolation_level_ != IsolationLevel::NON_TRANSACTIONAL) {
    YB_LOG_EVERY_N_SECS(DFATAL, 30)
        << "An attempt to perform a transactional operation on a non-transactional table: "
        << operation_->ToString();
  }

  docdb::PartialRangeKeyIntents partial_range_key_intents(metadata.UsePartialRangeKeyIntents());
  prepare_result_ = VERIFY_RESULT(docdb::PrepareDocWriteOperation(
      doc_ops_, write_batch.read_pairs(), tablet().metrics()->write_lock_latency,
      isolation_level_, kind(), row_mark_type, transactional_table,
      deadline(), partial_range_key_intents, tablet().shared_lock_manager()));

  TEST_SYNC_POINT("WriteQuery::DoExecute::PreparedDocWriteOps");

  auto* transaction_participant = tablet().transaction_participant();
  if (transaction_participant) {
    request_scope_ = RequestScope(transaction_participant);
  }

  if (!tablet().txns_enabled() || !transactional_table) {
    CompleteExecute();
    return Status::OK();
  }

  if (isolation_level_ == IsolationLevel::NON_TRANSACTIONAL) {
    auto now = tablet().clock()->Now();
    docdb::ResolveOperationConflicts(
        doc_ops_, now, tablet().doc_db(), partial_range_key_intents,
        transaction_participant, tablet().metrics()->transaction_conflicts.get(),
        [this, now](const Result<HybridTime>& result) {
          if (!result.ok()) {
            ExecuteDone(result.status());
            TRACE("InvokeCallback");
            return;
          }
          NonTransactionalConflictsResolved(now, *result);
          TRACE("NonTransactionalConflictsResolved");
        });
    return Status::OK();
  }

  if (isolation_level_ == IsolationLevel::SERIALIZABLE_ISOLATION &&
      prepare_result_.need_read_snapshot) {
    boost::container::small_vector<RefCntPrefix, 16> paths;
    for (const auto& doc_op : doc_ops_) {
      paths.clear();
      IsolationLevel ignored_isolation_level;
      RETURN_NOT_OK(doc_op->GetDocPaths(
          docdb::GetDocPathsMode::kLock, &paths, &ignored_isolation_level));
      for (const auto& path : paths) {
        auto key = path.as_slice();
        auto* pair = write_batch.mutable_read_pairs()->Add();
        pair->set_key(key.data(), key.size());
        // Empty values are disallowed by docdb.
        // https://github.com/YugaByte/yugabyte-db/issues/736
        pair->set_value(std::string(1, docdb::ValueTypeAsChar::kNullLow));
        write_batch.set_wait_policy(WAIT_ERROR);
      }
    }
  }

  docdb::ResolveTransactionConflicts(
      doc_ops_, write_batch, tablet().clock()->Now(),
      read_time_ ? read_time_.read : HybridTime::kMax,
      tablet().doc_db(), partial_range_key_intents,
      transaction_participant, tablet().metrics()->transaction_conflicts.get(),
      [this](const Result<HybridTime>& result) {
        if (!result.ok()) {
          ExecuteDone(result.status());
          TRACE("ExecuteDone");
          return;
        }
        TransactionalConflictsResolved();
        TRACE("TransactionalConflictsResolved");
      });

  return Status::OK();
}

void WriteQuery::NonTransactionalConflictsResolved(HybridTime now, HybridTime result) {
  if (now != result) {
    tablet().clock()->Update(result);
  }

  CompleteExecute();
}

void WriteQuery::TransactionalConflictsResolved() {
  auto status = DoTransactionalConflictsResolved();
  if (!status.ok()) {
    LOG(DFATAL) << status;
    ExecuteDone(status);
  }
}

CHECKED_STATUS WriteQuery::DoTransactionalConflictsResolved() {
  if (!read_time_) {
    auto safe_time = VERIFY_RESULT(tablet().SafeTime(RequireLease::kTrue));
    read_time_ = ReadHybridTime::FromHybridTimeRange(
        {safe_time, tablet().clock()->NowRange().second});
  } else if (prepare_result_.need_read_snapshot &&
             isolation_level_ == IsolationLevel::SERIALIZABLE_ISOLATION) {
    return STATUS_FORMAT(
        InvalidArgument,
        "Read time should NOT be specified for serializable isolation level: $0",
        read_time_);
  }

  CompleteExecute();
  return Status::OK();
}

void WriteQuery::CompleteExecute() {
  ExecuteDone(DoCompleteExecute());
}

CHECKED_STATUS WriteQuery::DoCompleteExecute() {
  auto read_op = prepare_result_.need_read_snapshot
      ? VERIFY_RESULT(ScopedReadOperation::Create(&tablet(), RequireLease::kTrue, read_time_))
      : ScopedReadOperation();
  // Actual read hybrid time used for read-modify-write operation.
  auto real_read_time = prepare_result_.need_read_snapshot
      ? read_op.read_time()
      // When need_read_snapshot is false, this time is used only to write TTL field of record.
      : ReadHybridTime::SingleTime(tablet().clock()->Now());

  // We expect all read operations for this transaction to be done in AssembleDocWriteBatch. Once
  // read_txn goes out of scope, the read point is deregistered.
  bool local_limit_updated = false;

  // This loop may be executed multiple times multiple times only for serializable isolation or
  // when read_time was not yet picked for snapshot isolation.
  // In all other cases it is executed only once.
  auto init_marker_behavior = tablet().table_type() == TableType::REDIS_TABLE_TYPE
      ? docdb::InitMarkerBehavior::kRequired
      : docdb::InitMarkerBehavior::kOptional;
  for (;;) {
    RETURN_NOT_OK(docdb::AssembleDocWriteBatch(
        doc_ops_, deadline(), real_read_time, tablet().doc_db(),
        request().mutable_write_batch(), init_marker_behavior,
        tablet().monotonic_counter(), &restart_read_ht_,
        tablet().metadata()->table_name()));

    // For serializable isolation we don't fix read time, so could do read restart locally,
    // instead of failing whole transaction.
    if (!restart_read_ht_.is_valid() || !allow_immediate_read_restart_) {
      break;
    }

    real_read_time.read = restart_read_ht_;
    if (!local_limit_updated) {
      local_limit_updated = true;
      real_read_time.local_limit = std::min(
          real_read_time.local_limit, VERIFY_RESULT(tablet().SafeTime(RequireLease::kTrue)));
    }

    restart_read_ht_ = HybridTime();

    request().mutable_write_batch()->clear_write_pairs();

    for (auto& doc_op : doc_ops_) {
      doc_op->ClearResponse();
    }
  }

  if (allow_immediate_read_restart_ &&
      isolation_level_ != IsolationLevel::NON_TRANSACTIONAL &&
      response_) {
    real_read_time.ToPB(response_->mutable_used_read_time());
  }

  if (restart_read_ht_.is_valid()) {
    return Status::OK();
  }

  docdb_locks_ = std::move(prepare_result_.lock_batch);

  return Status::OK();
}

Tablet& WriteQuery::tablet() const {
  return *operation_->tablet();
}

void WriteQuery::AdjustYsqlQueryTransactionality(size_t ysql_batch_size) {
  force_txn_path_ = ysql_batch_size > 0 && tablet().is_sys_catalog();
}

void WriteQuery::RedisExecuteDone(const Status& status) {
  if (!status.ok() || restart_read_ht().is_valid()) {
    StartSynchronization(std::move(self_), status);
    return;
  }
  for (auto& doc_op : doc_ops_) {
    auto* redis_write_operation = down_cast<docdb::RedisWriteOperation*>(doc_op.get());
    response_->add_redis_response_batch()->Swap(&redis_write_operation->response());
  }

  StartSynchronization(std::move(self_), Status::OK());
}

void WriteQuery::CqlExecuteDone(const Status& status) {
  if (restart_read_ht().is_valid()) {
    StartSynchronization(std::move(self_), Status::OK());
    return;
  }

  if (status.ok()) {
    UpdateQLIndexes();
  } else {
    CompleteQLWriteBatch(status);
  }
}

void WriteQuery::CompleteQLWriteBatch(const Status& status) {
  if (!status.ok()) {
    StartSynchronization(std::move(self_), status);
    return;
  }

  bool is_unique_index = tablet().metadata()->is_unique_index();

  for (auto& doc_op : doc_ops_) {
    std::unique_ptr<docdb::QLWriteOperation> ql_write_op(
        down_cast<docdb::QLWriteOperation*>(doc_op.release()));
    if (is_unique_index &&
        ql_write_op->request().type() == QLWriteRequestPB::QL_STMT_INSERT &&
        ql_write_op->response()->has_applied() && !ql_write_op->response()->applied()) {
      // If this is an insert into a unique index and it fails to apply, report duplicate value err.
      ql_write_op->response()->set_status(QLResponsePB::YQL_STATUS_USAGE_ERROR);
      ql_write_op->response()->set_error_message(
          Format("Duplicate value disallowed by unique index $0",
          tablet().metadata()->table_name()));
      DVLOG(1) << "Could not apply the given operation " << AsString(ql_write_op->request())
               << " due to " << AsString(ql_write_op->response());
    } else if (ql_write_op->rowblock() != nullptr) {
      // If the QL write op returns a rowblock, move the op to the transaction state to return the
      // rows data as a sidecar after the transaction completes.
      ql_write_ops_.emplace_back(std::move(ql_write_op));
    }
  }

  StartSynchronization(std::move(self_), Status::OK());
}

void WriteQuery::UpdateQLIndexes() {
  client::YBClient* client = nullptr;
  client::YBSessionPtr session;
  client::YBTransactionPtr txn;
  IndexOps index_ops;
  const ChildTransactionDataPB* child_transaction_data = nullptr;
  for (auto& doc_op : doc_ops_) {
    auto* write_op = down_cast<docdb::QLWriteOperation*>(doc_op.get());
    if (write_op->index_requests()->empty()) {
      continue;
    }
    if (!client) {
      client = &tablet().client();
      session = std::make_shared<client::YBSession>(client);
      session->SetDeadline(deadline());
      if (write_op->request().has_child_transaction_data()) {
        child_transaction_data = &write_op->request().child_transaction_data();
        if (!tablet().transaction_manager()) {
          StartSynchronization(
              std::move(self_),
              STATUS(Corruption, "Transaction manager is not present for index update"));
          return;
        }
        auto child_data = client::ChildTransactionData::FromPB(
            write_op->request().child_transaction_data());
        if (!child_data.ok()) {
          StartSynchronization(std::move(self_), child_data.status());
          return;
        }
        txn = std::make_shared<client::YBTransaction>(tablet().transaction_manager(), *child_data);
        session->SetTransaction(txn);
      } else {
        child_transaction_data = nullptr;
      }
    } else if (write_op->request().has_child_transaction_data()) {
      DCHECK_ONLY_NOTNULL(child_transaction_data);
      DCHECK_EQ(child_transaction_data->ShortDebugString(),
                write_op->request().child_transaction_data().ShortDebugString());
    } else {
      DCHECK(child_transaction_data == nullptr) <<
          "Value: " << child_transaction_data->ShortDebugString();
    }

    // Apply the write ops to update the index
    for (auto& pair : *write_op->index_requests()) {
      client::YBTablePtr index_table;
      bool cache_used_ignored = false;
      auto metadata_cache = tablet().YBMetaDataCache();
      if (!metadata_cache) {
        StartSynchronization(
            std::move(self_),
            STATUS(Corruption, "Table metadata cache is not present for index update"));
        return;
      }
      // TODO create async version of GetTable.
      // It is ok to have sync call here, because we use cache and it should not take too long.
      auto status = metadata_cache->GetTable(pair.first->table_id(), &index_table,
                                             &cache_used_ignored);
      if (!status.ok()) {
        StartSynchronization(std::move(self_), status);
        return;
      }
      std::shared_ptr<client::YBqlWriteOp> index_op(index_table->NewQLWrite());
      index_op->mutable_request()->Swap(&pair.second);
      index_op->mutable_request()->MergeFrom(pair.second);
      session->Apply(index_op);
      index_ops.emplace_back(std::move(index_op), write_op);
    }
  }

  if (!session) {
    CompleteQLWriteBatch(Status::OK());
    return;
  }

  session->FlushAsync(std::bind(
      &WriteQuery::UpdateQLIndexesFlushed, this, session, txn, std::move(index_ops), _1));
}

void WriteQuery::UpdateQLIndexesFlushed(
    const client::YBSessionPtr& session, const client::YBTransactionPtr& txn,
    const IndexOps& index_ops, client::FlushStatus* flush_status) {
  std::unique_ptr<WriteQuery> query(std::move(self_));

  const auto& status = flush_status->status;
  if (PREDICT_FALSE(!status.ok())) {
    // When any error occurs during the dispatching of YBOperation, YBSession saves the error and
    // returns IOError. When it happens, retrieves the errors and discard the IOError.
    if (status.IsIOError()) {
      for (const auto& error : flush_status->errors) {
        // return just the first error seen.
        Cancel(error->status());
        return;
      }
    }
    Cancel(status);
    return;
  }

  ChildTransactionResultPB child_result;
  if (txn) {
    auto finish_result = txn->FinishChild();
    if (!finish_result.ok()) {
      query->Cancel(finish_result.status());
      return;
    }
    child_result = std::move(*finish_result);
  }

  // Check the responses of the index write ops.
  for (const auto& pair : index_ops) {
    std::shared_ptr<client::YBqlWriteOp> index_op = pair.first;
    auto* response = pair.second->response();
    DCHECK_ONLY_NOTNULL(response);
    auto* index_response = index_op->mutable_response();

    if (index_response->status() != QLResponsePB::YQL_STATUS_OK) {
      DVLOG(1) << "Got status " << index_response->status() << " for " << AsString(index_op);
      response->set_status(index_response->status());
      response->set_error_message(std::move(*index_response->mutable_error_message()));
    }
    if (txn) {
      *response->mutable_child_transaction_result() = child_result;
    }
  }

  self_ = std::move(query);
  CompleteQLWriteBatch(Status::OK());
}

void WriteQuery::PgsqlExecuteDone(const Status& status) {
  if (!status.ok() || restart_read_ht_.is_valid()) {
    StartSynchronization(std::move(self_), status);
    return;
  }

  for (auto& doc_op : doc_ops_) {
    // We'll need to return the number of rows inserted, updated, or deleted by each operation.
    std::unique_ptr<docdb::PgsqlWriteOperation> pgsql_write_op(
        down_cast<docdb::PgsqlWriteOperation*>(doc_op.release()));
    pgsql_write_ops_.emplace_back(std::move(pgsql_write_op));
  }

  StartSynchronization(std::move(self_), Status::OK());
}

void WriteQuery::SimpleExecuteDone(const Status& status) {
  StartSynchronization(std::move(self_), status);
}

}  // namespace tablet
}  // namespace yb
