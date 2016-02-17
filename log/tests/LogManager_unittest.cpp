#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "transaction/Transaction.hpp"
#include <string.h>

#include "gtest/gtest.h"

namespace quickstep {

// Test if LSN increments as expected
TEST(LogManagerTest, LSNTest) {
  LogManager log_manager;
  EXPECT_EQ(log_manager.current_LSN_, (LSN) 1);
  EXPECT_EQ(log_manager.prev_LSN_, (LSN) 0);
  // Add first record
  LogRecord log_record_1((TransactionId) 1, LogRecord::LogRecordType::kEMPTY);
  log_manager.sendLogRequest(log_record_1);
  log_manager.fetchNext();
  EXPECT_EQ(log_manager.current_LSN_, (LSN) (1 + HEADER_LENGTH));
  EXPECT_EQ(log_manager.prev_LSN_, (LSN) 1);
  // Add second record 
  LogRecord log_record_2((TransactionId) 2, LogRecord::LogRecordType::kEMPTY);
  log_manager.sendLogRequest(log_record_2);
  log_manager.fetchNext();
  EXPECT_EQ(log_manager.current_LSN_, (LSN) (1 + HEADER_LENGTH * 2));
  EXPECT_EQ(log_manager.prev_LSN_, (LSN) (1 + HEADER_LENGTH));
}

// Test if buffer has the correct size (after writing and forcing to disk)
TEST(LogManagerTest, BufferSizeTest) {
  LogManager log_manager;
  EXPECT_EQ((int)log_manager.buffer_.length(), 0);
  // Write a record
  LogRecord log_record_1((TransactionId) 1, LogRecord::LogRecordType::kEMPTY);
  log_manager.sendLogRequest(log_record_1);
  EXPECT_EQ((int)log_manager.buffer_.length(), 0);
  log_manager.fetchNext();
  EXPECT_EQ((int)log_manager.buffer_.length(), HEADER_LENGTH);
  // Force to disk
  log_manager.forceToDisk("LOG");
  EXPECT_EQ((int)log_manager.buffer_.length(), 0);
}

// Test if the header is translated correctly
TEST(LogManagerTest, HeaderTranslationTest) {
  LogManager log_manager;
  // Record the record header
  LogRecord log_record_1((TransactionId) 1, LogRecord::LogRecordType::kEMPTY);
  std::uint64_t current_LSN_1 = log_manager.current_LSN_;
  std::uint64_t prev_LSN_1 = log_manager.prev_LSN_;
  std::uint64_t trans_prev_LSN_1 = log_manager.log_table_.getPrevLSN((TransactionId) 1);
  // Write a record
  log_manager.sendLogRequest(log_record_1);
  log_manager.fetchNext();
  // Check every fields' translations in the buffer
  std::string buffer = log_manager.buffer_;
  EXPECT_EQ(log_manager.strToInt(buffer.substr(LENGTH_START, LENGTH_END)), (int)buffer.length());
  EXPECT_EQ((int)buffer.at(TYPE_START), (int)LogRecord::LogRecordType::kEMPTY);
  EXPECT_EQ(log_manager.strToId(buffer.substr(TID_START, TID_END)), (TransactionId) 1);
  EXPECT_EQ(log_manager.strToId(buffer.substr(CURRENT_LSN_START, CURRENT_LSN_END)), current_LSN_1);
  EXPECT_EQ(log_manager.strToId(buffer.substr(PREV_LSN_START, CURRENT_LSN_END)), prev_LSN_1);
  EXPECT_EQ(log_manager.strToId(buffer.substr(TRANS_PREV_LSN_START, TRANS_PREV_LSN_END)), trans_prev_LSN_1);
}

} // namespace quickstep
