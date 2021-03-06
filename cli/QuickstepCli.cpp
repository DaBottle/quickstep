/**
 *   Copyright 2011-2015 Quickstep Technologies LLC.
 *   Copyright 2015 Pivotal Software, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 **/

/* A standalone command-line interface to QuickStep */

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <thread>  // NOLINT(build/c++11)


#include "cli/CliConfig.h"  // For QUICKSTEP_USE_LINENOISE.
#include "cli/DropRelation.hpp"

#ifdef QUICKSTEP_USE_LINENOISE
#include "cli/LineReaderLineNoise.hpp"
typedef quickstep::LineReaderLineNoise LineReaderImpl;
#else
#include "cli/LineReaderDumb.hpp"
typedef quickstep::LineReaderDumb LineReaderImpl;
#endif

#include "cli/InputParserUtil.hpp"
#include "cli/PrintToScreen.hpp"
#include "parser/ParseStatement.hpp"
#include "parser/SqlParserWrapper.hpp"
#include "query_execution/Foreman.hpp"
#include "query_execution/QueryExecutionTypedefs.hpp"
#include "query_execution/QueryContext.hpp"
#include "query_execution/Worker.hpp"
#include "query_execution/WorkerDirectory.hpp"
#include "query_execution/WorkerMessage.hpp"
#include "query_optimizer/QueryHandle.hpp"
#include "query_optimizer/QueryPlan.hpp"
#include "query_optimizer/QueryProcessor.hpp"
#include "storage/StorageConfig.h"  // For QUICKSTEP_HAVE_FILE_MANAGER_HDFS.

#ifdef QUICKSTEP_HAVE_FILE_MANAGER_HDFS
#include "storage/FileManagerHdfs.hpp"
#endif

#include "storage/PreloaderThread.hpp"
#include "threading/ThreadIDBasedMap.hpp"
#include "utility/Macros.hpp"
#include "utility/PtrVector.hpp"
#include "utility/SqlError.hpp"

#include "gflags/gflags.h"

#include "glog/logging.h"

#include "tmb/address.h"
#include "tmb/id_typedefs.h"
#include "tmb/message_bus.h"
#include "tmb/message_style.h"
#include "tmb/tagged_message.h"

namespace quickstep {
class CatalogRelation;
}

using std::fflush;
using std::fprintf;
using std::printf;
using std::string;
using std::vector;

using quickstep::Address;
using quickstep::CatalogRelation;
using quickstep::DropRelation;
using quickstep::Foreman;
using quickstep::InputParserUtil;
using quickstep::MessageBusImpl;
using quickstep::MessageStyle;
using quickstep::ParseResult;
using quickstep::ParseStatement;
using quickstep::PrintToScreen;
using quickstep::PtrVector;
using quickstep::QueryContext;
using quickstep::QueryHandle;
using quickstep::QueryPlan;
using quickstep::QueryProcessor;
using quickstep::SqlParserWrapper;
using quickstep::TaggedMessage;
using quickstep::Worker;
using quickstep::WorkerDirectory;
using quickstep::WorkerMessage;
using quickstep::kPoisonMessage;

using tmb::client_id;

namespace quickstep {

#ifdef QUICKSTEP_OS_WINDOWS
static constexpr char kPathSeparator = '\\';
static constexpr char kDefaultStoragePath[] = "qsstor\\";
#else
static constexpr char kPathSeparator = '/';
static constexpr char kDefaultStoragePath[] = "qsstor/";
#endif

DEFINE_int32(num_workers, 0, "Number of worker threads. If this value is "
                             "specified and is greater than 0, then this "
                             "user-supplied value is used. Else (i.e. the"
                             "default case), we examine the reported "
                             "hardware concurrency level, and use that.");
DEFINE_bool(preload_buffer_pool, false,
            "If true, pre-load all known blocks into buffer pool before "
            "accepting queries (should also set --buffer_pool_slots to be "
            "large enough to accomodate the entire database).");
DEFINE_string(storage_path, kDefaultStoragePath,
              "Filesystem path for quickstep database storage.");
DEFINE_string(worker_affinities, "",
              "A comma-separated list of CPU IDs to pin worker threads to "
              "(leaving this empty will cause all worker threads to inherit "
              "the affinity mask of the Quickstep process, which typically "
              "means that they will all be runable on any CPU according to "
              "the kernel's own scheduling policy).");
}  // namespace quickstep

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Detect the hardware concurrency level. Note this call will return 0
  // if it fails (which it may on some machines/environments).
  const unsigned int num_hw_threads = std::thread::hardware_concurrency();

  // Use the command-line value if that was supplied, else use the value
  // that we computed above, provided it did return a valid value.
  // TODO(jmp): May need to change this at some point to keep one thread
  //            available for the OS if the hardware concurrency level is high.
  const unsigned int real_num_workers = quickstep::FLAGS_num_workers != 0
                                      ? quickstep::FLAGS_num_workers
                                      : (num_hw_threads != 0 ?
                                         num_hw_threads
                                         : 1);

  if (real_num_workers > 0) {
    printf("Starting Quickstep with %d worker thread(s)\n", real_num_workers);
  } else {
    LOG(FATAL) << "Quickstep needs at least one worker thread";
  }

#ifdef QUICKSTEP_HAVE_FILE_MANAGER_HDFS
  if (quickstep::FLAGS_use_hdfs) {
    LOG(INFO) << "Using HDFS as the default persistent storage, with namenode at "
              << quickstep::FLAGS_hdfs_namenode_host << ":"
              << quickstep::FLAGS_hdfs_namenode_port << " and block replication factor "
              << quickstep::FLAGS_hdfs_num_replications << "\n";
  }
#endif

  // Initialize the thread ID based map here before the Foreman and workers are
  // constructed because the initialization isn't thread safe.
  quickstep::ClientIDMap::Instance();

  MessageBusImpl bus;
  bus.Initialize();

  // The TMB client id for the main thread, used to kill workers at the end.
  const client_id main_thread_client_id = bus.Connect();
  bus.RegisterClientAsSender(main_thread_client_id, kPoisonMessage);

  Foreman foreman(&bus);

  // Setup the paths used by StorageManager.
  string fixed_storage_path(quickstep::FLAGS_storage_path);
  if (!fixed_storage_path.empty()
      && (fixed_storage_path.back() != quickstep::kPathSeparator)) {
    fixed_storage_path.push_back(quickstep::kPathSeparator);
  }

  string catalog_path(fixed_storage_path);
  catalog_path.append("catalog.pb.bin");

  // Setup QueryProcessor, including CatalogDatabase and StorageManager.
  std::unique_ptr<QueryProcessor> query_processor;
  try {
    // TODO(zuyu): Remove passing 'bus' once WorkOrder serialization is done.
    query_processor.reset(new QueryProcessor(catalog_path, fixed_storage_path, foreman.getBusClientID(), &bus));
  } catch (const std::exception &e) {
    LOG(FATAL) << "FATAL ERROR DURING STARTUP: " << e.what();
  } catch (...) {
    LOG(FATAL) << "NON-STANDARD EXCEPTION DURING STARTUP";
  }

  // Parse the CPU affinities for workers and the preloader thread, if enabled
  // to warm up the buffer pool.
  const vector<int> worker_cpu_affinities =
      InputParserUtil::ParseWorkerAffinities(real_num_workers,
                                             quickstep::FLAGS_worker_affinities);

  if (quickstep::FLAGS_preload_buffer_pool) {
    quickstep::PreloaderThread preloader(*query_processor->getDefaultDatabase(),
                                         query_processor->getStorageManager(),
                                         worker_cpu_affinities.front());
    printf("Preloading buffer pool... ");
    fflush(stdout);
    preloader.start();
    preloader.join();
    printf("DONE\n");
  }

  // Get the NUMA affinities for workers.
  vector<int> cpu_numa_nodes = InputParserUtil::GetNUMANodesForCPUs();
  if (cpu_numa_nodes.empty()) {
    // libnuma is not present. Assign -1 as the NUMA node for every worker.
    cpu_numa_nodes.assign(worker_cpu_affinities.size(), -1);
  }

  vector<int> worker_numa_nodes;
  PtrVector<Worker> workers;
  vector<client_id> worker_client_ids;

  // TODO(zuyu): Construct QueryContext in Shiftboss to avoid Worker's access.
  std::unique_ptr<QueryContext> query_context;

  // Initialize the worker threads.
  DCHECK_EQ(static_cast<std::size_t>(real_num_workers),
            worker_cpu_affinities.size());
  for (std::size_t worker_idx = 0;
       worker_idx < worker_cpu_affinities.size();
       ++worker_idx) {
    int numa_node_id = -1;
    if (worker_cpu_affinities[worker_idx] >= 0) {
      // This worker can be NUMA affinitized.
      numa_node_id = cpu_numa_nodes[worker_cpu_affinities[worker_idx]];
    }
    worker_numa_nodes.push_back(numa_node_id);

    workers.push_back(new Worker(worker_idx,
                                 query_context,
                                 &bus,
                                 query_processor->getDefaultDatabase(),
                                 query_processor->getStorageManager(),
                                 worker_cpu_affinities[worker_idx]));
    worker_client_ids.push_back(workers.back().getBusClientID());
  }

  // TODO(zuyu): Move WorkerDirectory within Shiftboss once the latter is added.
  WorkerDirectory worker_directory(worker_cpu_affinities.size(),
                                   worker_client_ids,
                                   worker_numa_nodes);

  foreman.setWorkerDirectory(&worker_directory);

  // Start the worker threads.
  for (Worker &worker : workers) {
    worker.start();
  }

  LineReaderImpl line_reader("quickstep> ",
                             "      ...> ");
  SqlParserWrapper parser_wrapper;
  std::chrono::time_point<std::chrono::steady_clock> start, end;

  for (;;) {
    string *command_string = new string();
    *command_string = line_reader.getNextCommand();
    if (command_string->size() == 0) {
      delete command_string;
      break;
    }

    parser_wrapper.feedNextBuffer(command_string);

    bool quitting = false;
    for (;;) {
      ParseResult result = parser_wrapper.getNextStatement();
      if (result.condition == ParseResult::kSuccess) {
        if (result.parsed_statement->getStatementType() == ParseStatement::kQuit) {
          quitting = true;
          break;
        }

        std::unique_ptr<QueryHandle> query_handle;
        try {
          query_handle.reset(query_processor->generateQueryHandle(*result.parsed_statement));
        } catch (const quickstep::SqlError &sql_error) {
          fprintf(stderr, "%s", sql_error.formatMessage(*command_string).c_str());
          break;
        }

        DCHECK(query_handle->getQueryPlanMutable() != nullptr);
        foreman.setQueryPlan(query_handle->getQueryPlanMutable()->getQueryPlanDAGMutable());

        try {
          start = std::chrono::steady_clock::now();
          query_context.reset(new QueryContext(query_handle->getQueryContextProto(),
                                               query_processor->getDefaultDatabase(),
                                               query_processor->getStorageManager(),
                                               &bus));
          foreman.setQueryContext(query_context.get());
          foreman.start();
          foreman.join();
          end = std::chrono::steady_clock::now();

          const CatalogRelation *query_result_relation = query_handle->getQueryResultRelation();
          if (query_result_relation) {
            PrintToScreen::PrintRelation(*query_result_relation,
                                         query_processor->getStorageManager(),
                                         stdout);

            DropRelation::Drop(*query_result_relation,
                               query_processor->getDefaultDatabase(),
                               query_processor->getStorageManager());
          }

          query_processor->saveCatalog();
          printf("Execution time: %g seconds\n",
                 std::chrono::duration<double>(end - start).count());
        } catch (const std::exception &e) {
          fprintf(stderr, "QUERY EXECUTION ERROR: %s\n", e.what());
          break;
        }
        printf("Query Complete\n");
      } else {
        if (result.condition == ParseResult::kError) {
          fprintf(stderr, "%s", result.error_message.c_str());
        }
        break;
      }
    }

    if (quitting) {
      break;
    }
  }

  // Terminate all workers before exiting.
  // The main thread broadcasts poison message to the workers. Each worker dies
  // after receiving poison message. The order of workers' death is irrelavant.
  MessageStyle style;
  style.Broadcast(true);
  Address address;
  address.All(true);
  WorkerMessage poison_message(WorkerMessage::PoisonMessage());
  TaggedMessage poison_tagged_message;
  poison_tagged_message.set_message(
      &poison_message,
      sizeof(poison_message),
      quickstep::kPoisonMessage);

  bus.Send(main_thread_client_id,
           address,
           style,
           std::move(poison_tagged_message));

  for (Worker &worker : workers) {
    worker.join();
  }

  return 0;
}
