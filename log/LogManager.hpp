#ifndef QUICKSTEP_LOG_LOG_MANAGER_
#define QUICKSTEP_LOG_LOG_MANAGER_

#include <string.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"
#include "storage/TupleStorageSubBlock.hpp"
#include "utility/ThreadSafeQueue.hpp"
#include "gtest/gtest_prod.h"

namespace quickstep {

class LogManager {

// Header
// Header Format (37 bytes)
// length(4) type(1) tid(8) current_LSN(8) prev_LSN(8) trans_prev_LSN(8)
public:
  LogManager();

  // APIs for write a log
  // Log in-place update (if not in-place, will be logged as delete/re-insert)
  void logUpdate(TransactionId tid,
                 block_id bid,
                 tuple_id tupldId,
                 std::unordered_map<attribute_id, TypedValue>* old_value,
                 std::unordered_map<attribute_id, TypedValue>* updated_value);

  void logInsert(TransactionId tid,
                 block_id bid,
                 tuple_id tupleId,
                 Tuple* tuple);

  void logDelete(TransactionId tid,
                 block_id bid,
                 tuple_id tupleId,
                 Tuple* tuple);  

  // API for flush to disk
  void sendForceRequest();

  // Read private fields
  std::string getBuffer();

  LSN getCurrentLSN();

  LSN getPrevLSN();

  LSN getTransPrevLSN(TransactionId tid);

private:
  // fetch a log record from queue and write it to buffer
  void writeToBuffer(LogRecord* record);

  // Force the current buffer (given length) written to disk on the given file
  void flushToDisk(std::string filename);

  // write an empty log (only header)
  void logEmpty(TransactionId tid);

  LSN current_LSN_; // 32 bits for file index, 32 bits for offsets in a log file
  LSN prev_LSN_;
  LogTable log_table_;
  std::string buffer_;
  std::mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(LogManager);
};

} // namespace quickstep
#endif
