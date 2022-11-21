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

#pragma once

#include <memory>

#include "yb/gutil/ref_counted.h"

#include "yb/common/common_fwd.h"

#include "yb/util/strongly_typed_bool.h"

namespace google {
namespace protobuf {

class Message;

}
}

namespace yb {
namespace pggate {

class PgClient;

class PgTable;
class PgTableDesc;
using PgTableDescPtr = scoped_refptr<PgTableDesc>;

class PgsqlOp;
class PgsqlReadOp;
class PgsqlWriteOp;
using PgsqlOpPtr = std::shared_ptr<PgsqlOp>;
using PgsqlReadOpPtr = std::shared_ptr<PgsqlReadOp>;
using PgsqlWriteOpPtr = std::shared_ptr<PgsqlWriteOp>;
using PgsqlOps = std::vector<PgsqlOpPtr>;

YB_STRONGLY_TYPED_BOOL(Commit);

}  // namespace pggate
}  // namespace yb
