/*
   Copyright 2023 The Silkworm Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <silkworm/infra/concurrency/coroutine.hpp>

#include <boost/asio/awaitable.hpp>

#include <silkworm/node/db/mdbx.hpp>
#include <silkworm/silkrpc/ethdb/cursor.hpp>
#include <silkworm/silkrpc/ethdb/file/local_cursor.hpp>
#include <silkworm/silkrpc/ethdb/kv/cached_database.hpp>
#include <silkworm/silkrpc/ethdb/transaction.hpp>

namespace silkworm::rpc::ethdb::file {

class LocalTransaction : public Transaction {
  public:
    explicit LocalTransaction(std::shared_ptr<mdbx::env_managed> chaindata_env)
        : chaindata_env_{std::move(chaindata_env)}, last_cursor_id_{0}, txn_{*chaindata_env_} {}

    ~LocalTransaction() override = default;

    [[nodiscard]] uint64_t view_id() const override { return txn_.id(); }

    boost::asio::awaitable<void> open() override;

    boost::asio::awaitable<std::shared_ptr<Cursor>> cursor(const std::string& table) override;

    boost::asio::awaitable<std::shared_ptr<CursorDupSort>> cursor_dup_sort(const std::string& table) override;

    std::shared_ptr<silkworm::State> create_state(boost::asio::any_io_executor& executor, const DatabaseReader& db_reader, uint64_t block_number) override;

    std::shared_ptr<node::ChainStorage> create_storage(const DatabaseReader& db_reader, ethbackend::BackEnd* backend) override;

    boost::asio::awaitable<void> close() override;

  private:
    boost::asio::awaitable<std::shared_ptr<CursorDupSort>> get_cursor(const std::string& table, bool is_cursor_dup_sort);

    std::map<std::string, std::shared_ptr<CursorDupSort>> cursors_;
    std::map<std::string, std::shared_ptr<CursorDupSort>> dup_cursors_;

    std::shared_ptr<mdbx::env_managed> chaindata_env_;
    uint32_t last_cursor_id_;
    db::ROTxn txn_;
};

}  // namespace silkworm::rpc::ethdb::file
