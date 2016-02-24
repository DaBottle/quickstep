#ifndef QUICKSTEP_LOG_LOG_MANAGER_
#define QUICKSTEP_LOG_LOG_MANAGER_

#include <string.h>
#include <atomic>
#include <cstdint>
#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"
#include "utility/ThreadSafeQueue.hpp"
#include "gtest/gtest_prod.h"

namespace quickstep {

class LogManager {

// Header
// Header Format (37 bytes)
// length(4) type(1) tid(8) current_LSN(8) prev_LSN(8) trans_prev_LSN(8)
public:
  LogManager();

  // Push the given log record into in_queue_
  void sendLogRequest(LogRecord* log_record);

  // fetch a log record from queue and write it to buffer
  void fetchNext();

  // Let the log manager know that a force-to-disk is needed
  void sendForceRequest();

private:
  // Force the current buffer (given length) written to disk on the given file
  void forceToDisk(std::string filename);

  // For debug
  // return the current header into a readable result
  void printHeader();

  std::atomic<LSN> current_LSN_; // 32 bits for file index, 32 bits for offsets in a log file
  LSN prev_LSN_;
  LogTable log_table_;
  ThreadSafeQueue<LogRecord*> in_queue_;
  std::uint64_t record_count_; // Count the number of record that has been written to buffer
  std::string buffer_;  

  // Friend to unit test
  FRIEND_TEST(LogManagerTest, BufferSizeTest);
  FRIEND_TEST(LogManagerTest, LSNTest);
  FRIEND_TEST(LogManagerTest, HeaderTranslationTest);
  FRIEND_TEST(LogManagerTest, UpdateTest);
  FRIEND_TEST(LogManagerTest, CommitAndAbortTest);

  DISALLOW_COPY_AND_ASSIGN(LogManager);
};

} // namespace quickstep
#endif
