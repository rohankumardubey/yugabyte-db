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

#ifndef YB_YQL_PGGATE_PGGATE_H_
#define YB_YQL_PGGATE_PGGATE_H_

#include <functional>
#include <memory>
#include <unordered_map>

#include "yb/client/async_initializer.h"
#include "yb/client/client_fwd.h"

#include "yb/common/pg_types.h"
#include "yb/common/transaction.h"

#include "yb/gutil/casts.h"
#include "yb/gutil/ref_counted.h"

#include "yb/rpc/rpc_fwd.h"

#include "yb/server/hybrid_clock.h"
#include "yb/server/server_base_options.h"

#include "yb/tserver/tserver_util_fwd.h"

#include "yb/util/mem_tracker.h"
#include "yb/util/metrics.h"
#include "yb/util/result.h"
#include "yb/util/status.h"
#include "yb/util/status_fwd.h"

#include "yb/yql/pggate/pg_client.h"
#include "yb/yql/pggate/pg_env.h"
#include "yb/yql/pggate/pg_expr.h"
#include "yb/yql/pggate/pg_gate_fwd.h"
#include "yb/yql/pggate/pg_statement.h"
#include "yb/yql/pggate/ybc_pg_typedefs.h"

namespace yb {
namespace pggate {
class PgSysTablePrefetcher;
class PgSession;

//--------------------------------------------------------------------------------------------------

struct PgApiContext {
  struct MessengerHolder {
    std::unique_ptr<rpc::SecureContext> security_context;
    std::unique_ptr<rpc::Messenger> messenger;

    MessengerHolder(
        std::unique_ptr<rpc::SecureContext> security_context,
        std::unique_ptr<rpc::Messenger> messenger);
    MessengerHolder(MessengerHolder&&);

    ~MessengerHolder();
  };

  std::unique_ptr<MetricRegistry> metric_registry;
  scoped_refptr<MetricEntity> metric_entity;
  std::shared_ptr<MemTracker> mem_tracker;
  MessengerHolder messenger_holder;
  std::unique_ptr<rpc::ProxyCache> proxy_cache;

  PgApiContext();
  PgApiContext(PgApiContext&&);
  ~PgApiContext();
};

//--------------------------------------------------------------------------------------------------
// Implements support for CAPI.
class PgApiImpl {
 public:
  PgApiImpl(PgApiContext context, const YBCPgTypeEntity *YBCDataTypeTable, int count,
            YBCPgCallbacks pg_callbacks);
  ~PgApiImpl();

  const YBCPgCallbacks* pg_callbacks() {
    return &pg_callbacks_;
  }

  void ResetCatalogReadTime();

  // Initialize ENV within which PGSQL calls will be executed.
  Status CreateEnv(PgEnv **pg_env);
  Status DestroyEnv(PgEnv *pg_env);

  // Initialize a session to process statements that come from the same client connection.
  // If database_name is empty, a session is created without connecting to any database.
  Status InitSession(const PgEnv *pg_env, const std::string& database_name);

  // YB Memctx: Create, Destroy, and Reset must be "static" because a few contexts are created
  //            before YugaByte environments including PgGate are created and initialized.
  // Create YB Memctx. Each memctx will be associated with a Postgres's MemoryContext.
  static PgMemctx *CreateMemctx();
  // Destroy YB Memctx.
  static Status DestroyMemctx(PgMemctx *memctx);
  // Reset YB Memctx.
  static Status ResetMemctx(PgMemctx *memctx);
  // Cache statements in YB Memctx. When Memctx is destroyed, the statement is destructed.
  Status AddToCurrentPgMemctx(std::unique_ptr<PgStatement> stmt,
                                      PgStatement **handle);
  // Cache table descriptor in YB Memctx. When Memctx is destroyed, the descriptor is destructed.
  Status AddToCurrentPgMemctx(size_t table_desc_id,
                                      const PgTableDescPtr &table_desc);
  // Read table descriptor that was cached in YB Memctx.
  Status GetTabledescFromCurrentPgMemctx(size_t table_desc_id, PgTableDesc **handle);

  // Invalidate the sessions table cache.
  Status InvalidateCache();

  // Get the gflag TEST_ysql_disable_transparent_cache_refresh_retry.
  bool GetDisableTransparentCacheRefreshRetry();

  Result<bool> IsInitDbDone();

  Result<uint64_t> GetSharedCatalogVersion();
  Result<uint64_t> GetSharedAuthKey();

  // Setup the table to store sequences data.
  Status CreateSequencesDataTable();

  Status InsertSequenceTuple(int64_t db_oid,
                                     int64_t seq_oid,
                                     uint64_t ysql_catalog_version,
                                     int64_t last_val,
                                     bool is_called);

  Status UpdateSequenceTupleConditionally(int64_t db_oid,
                                                  int64_t seq_oid,
                                                  uint64_t ysql_catalog_version,
                                                  int64_t last_val,
                                                  bool is_called,
                                                  int64_t expected_last_val,
                                                  bool expected_is_called,
                                                  bool *skipped);

  Status UpdateSequenceTuple(int64_t db_oid,
                                     int64_t seq_oid,
                                     uint64_t ysql_catalog_version,
                                     int64_t last_val,
                                     bool is_called,
                                     bool* skipped);

  Status ReadSequenceTuple(int64_t db_oid,
                                   int64_t seq_oid,
                                   uint64_t ysql_catalog_version,
                                   int64_t *last_val,
                                   bool *is_called);

  Status DeleteSequenceTuple(int64_t db_oid, int64_t seq_oid);

  void DeleteStatement(PgStatement *handle);

  // Search for type_entity.
  const YBCPgTypeEntity *FindTypeEntity(int type_oid);

  //------------------------------------------------------------------------------------------------
  // Connect database. Switch the connected database to the given "database_name".
  Status ConnectDatabase(const char *database_name);

  // Determine whether the given database is colocated.
  Status IsDatabaseColocated(const PgOid database_oid, bool *colocated);

  // Create database.
  Status NewCreateDatabase(const char *database_name,
                                   PgOid database_oid,
                                   PgOid source_database_oid,
                                   PgOid next_oid,
                                   const bool colocated,
                                   PgStatement **handle);
  Status ExecCreateDatabase(PgStatement *handle);

  // Drop database.
  Status NewDropDatabase(const char *database_name,
                                 PgOid database_oid,
                                 PgStatement **handle);
  Status ExecDropDatabase(PgStatement *handle);

  // Alter database.
  Status NewAlterDatabase(const char *database_name,
                                 PgOid database_oid,
                                 PgStatement **handle);
  Status AlterDatabaseRenameDatabase(PgStatement *handle, const char *newname);
  Status ExecAlterDatabase(PgStatement *handle);

  // Reserve oids.
  Status ReserveOids(PgOid database_oid,
                             PgOid next_oid,
                             uint32_t count,
                             PgOid *begin_oid,
                             PgOid *end_oid);

  Status GetCatalogMasterVersion(uint64_t *version);

  // Load table.
  Result<PgTableDescPtr> LoadTable(const PgObjectId& table_id);

  // Invalidate the cache entry corresponding to table_id from the PgSession table cache.
  void InvalidateTableCache(const PgObjectId& table_id);

  //------------------------------------------------------------------------------------------------
  // Create and drop tablegroup.

  Status NewCreateTablegroup(const char *database_name,
                                     const PgOid database_oid,
                                     const PgOid tablegroup_oid,
                                     const PgOid tablespace_oid,
                                     PgStatement **handle);

  Status ExecCreateTablegroup(PgStatement *handle);

  Status NewDropTablegroup(const PgOid database_oid,
                                   const PgOid tablegroup_oid,
                                   PgStatement **handle);

  Status ExecDropTablegroup(PgStatement *handle);

  //------------------------------------------------------------------------------------------------
  // Create, alter and drop table.
  Status NewCreateTable(const char *database_name,
                                const char *schema_name,
                                const char *table_name,
                                const PgObjectId& table_id,
                                bool is_shared_table,
                                bool if_not_exist,
                                bool add_primary_key,
                                bool is_colocated_via_database,
                                const PgObjectId& tablegroup_oid,
                                const ColocationId colocation_id,
                                const PgObjectId& tablespace_oid,
                                const PgObjectId& matview_pg_table_oid,
                                PgStatement **handle);

  Status CreateTableAddColumn(PgStatement *handle, const char *attr_name, int attr_num,
                                      const YBCPgTypeEntity *attr_type, bool is_hash,
                                      bool is_range, bool is_desc, bool is_nulls_first);

  Status CreateTableSetNumTablets(PgStatement *handle, int32_t num_tablets);

  Status AddSplitBoundary(PgStatement *handle, PgExpr **exprs, int expr_count);

  Status ExecCreateTable(PgStatement *handle);

  Status NewAlterTable(const PgObjectId& table_id,
                               PgStatement **handle);

  Status AlterTableAddColumn(PgStatement *handle, const char *name,
                                     int order, const YBCPgTypeEntity *attr_type);

  Status AlterTableRenameColumn(PgStatement *handle, const char *oldname,
                                        const char *newname);

  Status AlterTableDropColumn(PgStatement *handle, const char *name);

  Status AlterTableRenameTable(PgStatement *handle, const char *db_name,
                                       const char *newname);

  Status ExecAlterTable(PgStatement *handle);

  Status NewDropTable(const PgObjectId& table_id,
                              bool if_exist,
                              PgStatement **handle);

  Status NewTruncateTable(const PgObjectId& table_id,
                                  PgStatement **handle);

  Status ExecTruncateTable(PgStatement *handle);

  Status GetTableDesc(const PgObjectId& table_id,
                              PgTableDesc **handle);

  Result<YBCPgColumnInfo> GetColumnInfo(YBCPgTableDesc table_desc,
                                        int16_t attr_number);

  Status DmlModifiesRow(PgStatement *handle, bool *modifies_row);

  Status SetIsSysCatalogVersionChange(PgStatement *handle);

  Status SetCatalogCacheVersion(PgStatement *handle, uint64_t catalog_cache_version);

  //------------------------------------------------------------------------------------------------
  // Create and drop index.
  Status NewCreateIndex(const char *database_name,
                                const char *schema_name,
                                const char *index_name,
                                const PgObjectId& index_id,
                                const PgObjectId& table_id,
                                bool is_shared_index,
                                bool is_unique_index,
                                const bool skip_index_backfill,
                                bool if_not_exist,
                                const PgObjectId& tablegroup_oid,
                                const YBCPgOid& colocation_id,
                                const PgObjectId& tablespace_oid,
                                PgStatement **handle);

  Status CreateIndexAddColumn(PgStatement *handle, const char *attr_name, int attr_num,
                                      const YBCPgTypeEntity *attr_type, bool is_hash,
                                      bool is_range, bool is_desc, bool is_nulls_first);

  Status CreateIndexSetNumTablets(PgStatement *handle, int32_t num_tablets);

  Status CreateIndexAddSplitRow(PgStatement *handle, int num_cols,
                                        YBCPgTypeEntity **types, uint64_t *data);

  Status ExecCreateIndex(PgStatement *handle);

  Status NewDropIndex(const PgObjectId& index_id,
                              bool if_exist,
                              PgStatement **handle);

  Status ExecPostponedDdlStmt(PgStatement *handle);

  Status BackfillIndex(const PgObjectId& table_id);

  //------------------------------------------------------------------------------------------------
  // All DML statements
  Status DmlAppendTarget(PgStatement *handle, PgExpr *expr);

  Status DmlAppendQual(PgStatement *handle, PgExpr *expr);

  Status DmlAppendColumnRef(PgStatement *handle, PgExpr *colref);

  // Binding Columns: Bind column with a value (expression) in a statement.
  // + This API is used to identify the rows you want to operate on. If binding columns are not
  //   there, that means you want to operate on all rows (full scan). You can view this as a
  //   a definitions of an initial rowset or an optimization over full-scan.
  //
  // + There are some restrictions on when BindColumn() can be used.
  //   Case 1: INSERT INTO tab(x) VALUES(x_expr)
  //   - BindColumn() can be used for BOTH primary-key and regular columns.
  //   - This bind-column function is used to bind "x" with "x_expr", and "x_expr" that can contain
  //     bind-variables (placeholders) and constants whose values can be updated for each execution
  //     of the same allocated statement.
  //
  //   Case 2: SELECT / UPDATE / DELETE <WHERE key = "key_expr">
  //   - BindColumn() can only be used for primary-key columns.
  //   - This bind-column function is used to bind the primary column "key" with "key_expr" that can
  //     contain bind-variables (placeholders) and constants whose values can be updated for each
  //     execution of the same allocated statement.
  Status DmlBindColumn(YBCPgStatement handle, int attr_num, YBCPgExpr attr_value);
  Status DmlBindColumnCondBetween(YBCPgStatement handle, int attr_num, YBCPgExpr attr_value,
      YBCPgExpr attr_value_end);
  Status DmlBindColumnCondIn(YBCPgStatement handle, int attr_num, int n_attr_values,
      YBCPgExpr *attr_value);

  Status DmlBindHashCode(PgStatement *handle, bool start_valid,
                                bool start_inclusive, uint64_t start_hash_val,
                                bool end_valid, bool end_inclusive,
                                uint64_t end_hash_val);

  Status DmlAddRowUpperBound(YBCPgStatement handle,
                                    int n_col_values,
                                    YBCPgExpr *col_values,
                                    bool is_inclusive);

  Status DmlAddRowLowerBound(YBCPgStatement handle,
                                    int n_col_values,
                                    YBCPgExpr *col_values,
                                    bool is_inclusive);

  // Binding Tables: Bind the whole table in a statement.  Do not use with BindColumn.
  Status DmlBindTable(YBCPgStatement handle);

  // Utility method to get the info for column 'attr_num'.
  Result<YBCPgColumnInfo> DmlGetColumnInfo(YBCPgStatement handle, int attr_num);

  // API for SET clause.
  Status DmlAssignColumn(YBCPgStatement handle, int attr_num, YBCPgExpr attr_value);

  // This function is to fetch the targets in YBCPgDmlAppendTarget() from the rows that were defined
  // by YBCPgDmlBindColumn().
  Status DmlFetch(PgStatement *handle, int32_t natts, uint64_t *values, bool *isnulls,
                          PgSysColumns *syscols, bool *has_data);

  // Utility method that checks stmt type and calls exec insert, update, or delete internally.
  Status DmlExecWriteOp(PgStatement *handle, int32_t *rows_affected_count);

  // This function adds a primary column to be used in the construction of the tuple id (ybctid).
  Status DmlAddYBTupleIdColumn(PgStatement *handle, int attr_num, uint64_t datum,
                                       bool is_null, const YBCPgTypeEntity *type_entity);

  using YBTupleIdProcessor = std::function<Status(const Slice&)>;
  Status ProcessYBTupleId(const YBCPgYBTupleIdDescriptor& descr,
                                  const YBTupleIdProcessor& processor);

  // DB Operations: SET, WHERE, ORDER_BY, GROUP_BY, etc.
  // + The following operations are run by DocDB.
  //   - API for "set_clause" (not yet implemented).
  //
  // + The following operations are run by Postgres layer. An API might be added to move these
  //   operations to DocDB.
  //   - API for "where_expr"
  //   - API for "order_by_expr"
  //   - API for "group_by_expr"

  // Buffer write operations.
  Status StartOperationsBuffering();
  Status StopOperationsBuffering();
  void ResetOperationsBuffering();
  Status FlushBufferedOperations();

  //------------------------------------------------------------------------------------------------
  // Insert.
  Status NewInsert(const PgObjectId& table_id,
                           bool is_single_row_txn,
                           bool is_region_local,
                           PgStatement **handle);

  Status ExecInsert(PgStatement *handle);

  Status InsertStmtSetUpsertMode(PgStatement *handle);

  Status InsertStmtSetWriteTime(PgStatement *handle, const HybridTime write_time);

  Status InsertStmtSetIsBackfill(PgStatement *handle, const bool is_backfill);

  //------------------------------------------------------------------------------------------------
  // Update.
  Status NewUpdate(const PgObjectId& table_id,
                           bool is_single_row_txn,
                           bool is_region_local,
                           PgStatement **handle);

  Status ExecUpdate(PgStatement *handle);

  //------------------------------------------------------------------------------------------------
  // Delete.
  Status NewDelete(const PgObjectId& table_id,
                           bool is_single_row_txn,
                           bool is_region_local,
                           PgStatement **handle);

  Status ExecDelete(PgStatement *handle);

  Status DeleteStmtSetIsPersistNeeded(PgStatement *handle, const bool is_persist_needed);

  //------------------------------------------------------------------------------------------------
  // Colocated Truncate.
  Status NewTruncateColocated(const PgObjectId& table_id,
                                      bool is_single_row_txn,
                                      bool is_region_local,
                                      PgStatement **handle);

  Status ExecTruncateColocated(PgStatement *handle);

  //------------------------------------------------------------------------------------------------
  // Select.
  Status NewSelect(const PgObjectId& table_id,
                           const PgObjectId& index_id,
                           const PgPrepareParameters *prepare_params,
                           bool is_region_local,
                           PgStatement **handle);

  Status SetForwardScan(PgStatement *handle, bool is_forward_scan);

  Status ExecSelect(PgStatement *handle, const PgExecParameters *exec_params);

  //------------------------------------------------------------------------------------------------
  // Analyze.
  Status NewSample(const PgObjectId& table_id,
                           const int targrows,
                           bool is_region_local,
                           PgStatement **handle);

  Status InitRandomState(PgStatement *handle, double rstate_w, uint64 rand_state);

  Status SampleNextBlock(PgStatement *handle, bool *has_more);

  Status ExecSample(PgStatement *handle);

  Status GetEstimatedRowCount(PgStatement *handle, double *liverows, double *deadrows);

  //------------------------------------------------------------------------------------------------
  // Transaction control.
  Status BeginTransaction();
  Status RecreateTransaction();
  Status RestartTransaction();
  Status ResetTransactionReadPoint();
  Status RestartReadPoint();
  Status CommitTransaction();
  Status AbortTransaction();
  Status SetTransactionIsolationLevel(int isolation);
  Status SetTransactionReadOnly(bool read_only);
  Status SetTransactionDeferrable(bool deferrable);
  Status EnableFollowerReads(bool enable_follower_reads, int32_t staleness_ms);
  Status EnterSeparateDdlTxnMode();
  bool HasWriteOperationsInDdlTxnMode() const;
  Status ExitSeparateDdlTxnMode();
  void ClearSeparateDdlTxnMode();
  Status SetActiveSubTransaction(SubTransactionId id);
  Status RollbackSubTransaction(SubTransactionId id);

  //------------------------------------------------------------------------------------------------
  // Expressions.
  // Column reference.
  Status NewColumnRef(
      PgStatement *handle, int attr_num, const PgTypeEntity *type_entity,
      bool collate_is_valid_non_c, const PgTypeAttrs *type_attrs, PgExpr **expr_handle);

  // Constant expressions.
  Status NewConstant(
      YBCPgStatement stmt, const YBCPgTypeEntity *type_entity, bool collate_is_valid_non_c,
      const char *collation_sortkey, uint64_t datum, bool is_null, YBCPgExpr *expr_handle);
  Status NewConstantVirtual(
      YBCPgStatement stmt, const YBCPgTypeEntity *type_entity,
      YBCPgDatumKind datum_kind, YBCPgExpr *expr_handle);
  Status NewConstantOp(
      YBCPgStatement stmt, const YBCPgTypeEntity *type_entity, bool collate_is_valid_non_c,
      const char *collation_sortkey, uint64_t datum, bool is_null, YBCPgExpr *expr_handle,
      bool is_gt);

  // TODO(neil) UpdateConstant should be merged into one.
  // Update constant.
  template<typename value_type>
  Status UpdateConstant(PgExpr *expr, value_type value, bool is_null) {
    if (expr->opcode() != PgExpr::Opcode::PG_EXPR_CONSTANT) {
      // Invalid handle.
      return STATUS(InvalidArgument, "Invalid expression handle for constant");
    }
    down_cast<PgConstant*>(expr)->UpdateConstant(value, is_null);
    return Status::OK();
  }
  Status UpdateConstant(PgExpr *expr, const char *value, bool is_null);
  Status UpdateConstant(PgExpr *expr, const void *value, int64_t bytes, bool is_null);

  // Operators.
  Status NewOperator(
      PgStatement *stmt, const char *opname, const YBCPgTypeEntity *type_entity,
      bool collate_is_valid_non_c, PgExpr **op_handle);
  Status OperatorAppendArg(PgExpr *op_handle, PgExpr *arg);

  // Foreign key reference caching.
  void DeleteForeignKeyReference(PgOid table_id, const Slice& ybctid);
  void AddForeignKeyReference(PgOid table_id, const Slice& ybctid);
  Result<bool> ForeignKeyReferenceExists(PgOid table_id, const Slice& ybctid, PgOid database_id);
  void AddForeignKeyReferenceIntent(PgOid table_id, bool is_region_local, const Slice& ybctid);

  // Sets the specified timeout in the rpc service.
  void SetTimeout(int timeout_ms);

  Result<client::TabletServersInfo> ListTabletServers();

  void StartSysTablePrefetching();
  void StopSysTablePrefetching();
  void RegisterSysTableForPrefetching(const PgObjectId& table_id, const PgObjectId& index_id);

  //------------------------------------------------------------------------------------------------
  // System Validation.
  Status ValidatePlacement(const char *placement_info);

 private:
  // Metrics.
  std::unique_ptr<MetricRegistry> metric_registry_;
  scoped_refptr<MetricEntity> metric_entity_;

  // Memory tracker.
  std::shared_ptr<MemTracker> mem_tracker_;

  PgApiContext::MessengerHolder messenger_holder_;

  std::unique_ptr<rpc::ProxyCache> proxy_cache_;

  // TODO Rename to client_ when YBClient is removed.
  PgClient pg_client_;

  // TODO(neil) Map for environments (we should have just one ENV?). Environments should contain
  // all the custom flags the PostgreSQL sets. We ignore them all for now.
  PgEnv::SharedPtr pg_env_;

  scoped_refptr<server::HybridClock> clock_;

  // Local tablet-server shared memory segment handle.
  std::unique_ptr<tserver::TServerSharedObject> tserver_shared_object_;

  YBCPgCallbacks pg_callbacks_;

  scoped_refptr<PgTxnManager> pg_txn_manager_;

  // Mapping table of YugaByte and PostgreSQL datatypes.
  std::unordered_map<int, const YBCPgTypeEntity *> type_map_;

  scoped_refptr<PgSession> pg_session_;
  std::unique_ptr<PgSysTablePrefetcher> pg_sys_table_prefetcher_;
};

}  // namespace pggate
}  // namespace yb

#endif // YB_YQL_PGGATE_PGGATE_H_
