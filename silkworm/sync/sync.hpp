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

#include <memory>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <silkworm/core/chain/config.hpp>
#include <silkworm/infra/common/log.hpp>
#include <silkworm/infra/grpc/client/client_context_pool.hpp>
#include <silkworm/node/db/mdbx.hpp>
#include <silkworm/node/stagedsync/client.hpp>
#include <silkworm/sentry/api/common/sentry_client.hpp>
#include <silkworm/silkrpc/daemon.hpp>

#include "block_exchange.hpp"
#include "chain_sync.hpp"
#include "sentry_client.hpp"

namespace silkworm::chainsync {

struct EngineRpcSettings {
    std::string engine_end_point{kDefaultEngineEndPoint};
    std::string private_api_addr{kDefaultPrivateApiAddr};
    log::Level log_verbosity{log::Level::kInfo};
    concurrency::WaitMode wait_mode{concurrency::WaitMode::blocking};
    std::string jwt_secret_file;
};

class Sync {
  public:
    Sync(boost::asio::io_context& io_context,
         mdbx::env_managed& chaindata_env,
         execution::Client& execution,
         const std::shared_ptr<silkworm::sentry::api::SentryClient>& sentry_client,
         const ChainConfig& config,
         const EngineRpcSettings& rpc_settings = {});

    Sync(const Sync&) = delete;
    Sync& operator=(const Sync&) = delete;

    //! Force PoW sync independently from chain config
    // TODO(canepat) remove when PoS sync works
    void force_pow(execution::Client& execution);

    boost::asio::awaitable<void> async_run();

  private:
    boost::asio::awaitable<void> run_tasks();
    boost::asio::awaitable<void> start_sync_sentry_client();
    boost::asio::awaitable<void> start_block_exchange();
    boost::asio::awaitable<void> start_chain_sync();
    boost::asio::awaitable<void> start_engine_rpc_server();

    //! The Sentry synchronous (i.e. blocking) client used by BlockExchange
    SentryClient sync_sentry_client_;

    //! The gateway for exchanging blocks with peers
    BlockExchange block_exchange_;

    //! The chain synchronization algorithm
    std::unique_ptr<ChainSync> chain_sync_;

    //! The Execution Layer Engine API RPC server
    std::unique_ptr<rpc::Daemon> engine_rpc_server_;
};

}  // namespace silkworm::chainsync
