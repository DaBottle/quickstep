#include "log/Helper.hpp"
#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
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
  log_manager.sendLogRequest(&log_record_1);
  log_manager.fetchNext();
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH), log_manager.current_LSN_);
  EXPECT_EQ((LSN) 1, log_manager.prev_LSN_);
  // Add second record 
  LogRecord log_record_2((TransactionId) 2, LogRecord::LogRecordType::kEMPTY);
  log_manager.sendLogRequest(&log_record_2);
  log_manager.fetchNext();
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH * 2), log_manager.current_LSN_);
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH), log_manager.prev_LSN_);
}

// Test if buffer has the correct size (after writing and forcing to disk)
TEST(LogManagerTest, BufferSizeTest) {
  LogManager log_manager;
  EXPECT_EQ((int)log_manager.buffer_.length(), 0);
  // Write a record
  LogRecord log_record_1((TransactionId) 1, LogRecord::LogRecordType::kEMPTY);
  log_manager.sendLogRequest(&log_record_1);
  EXPECT_EQ(0, (int)log_manager.buffer_.length());
  log_manager.fetchNext();
  EXPECT_EQ(Macros::kHEADER_LENGTH + 0, (int)log_manager.buffer_.length());
  // Force to disk
  log_manager.forceToDisk("LOG");
  EXPECT_EQ(0, (int)log_manager.buffer_.length());
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
  log_manager.sendLogRequest(&log_record_1);
  log_manager.fetchNext();
  // Check every fields' translation in the buffer
  std::string buffer = log_manager.buffer_;
  EXPECT_EQ((int)buffer.length(), Helper::strToInt(buffer.substr(Macros::kLENGTH_START, sizeof(int))));
  EXPECT_EQ((int)LogRecord::LogRecordType::kEMPTY, (int)buffer.at(Macros::kTYPE_START));
  EXPECT_EQ((TransactionId) 1, Helper::strToId(buffer.substr(Macros::kTID_START, sizeof(TransactionId))));
  EXPECT_EQ(current_LSN_1, Helper::strToId(buffer.substr(Macros::kCURRENT_LSN_START, sizeof(LSN))));
  EXPECT_EQ(prev_LSN_1, Helper::strToId(buffer.substr(Macros::kPREV_LSN_START, sizeof(LSN))));
  EXPECT_EQ(trans_prev_LSN_1, Helper::strToId(buffer.substr(Macros::kTRANS_PREV_LSN_START, sizeof(LSN))));
}

TEST(LogManagerTest, PayloadTranslationTest) {
  LogManager log_manager;
  // Check update log
  // Number update
  UpdateLogRecord update_log_record_1((TransactionId) 1, LogRecord::LogRecordType::kUPDATE, 12, 34, (block_id) 4, (tuple_id) 6, (attribute_id) 8);
  log_manager.sendLogRequest(&update_log_record_1);
  log_manager.fetchNext();
  
  std::string update_payload_1 = log_manager.buffer_.substr(Macros::kHEADER_LENGTH);
  EXPECT_EQ((block_id) 4, Helper::strToId(update_payload_1.substr(Macros::kBID_START, sizeof(block_id))));
  EXPECT_EQ((tuple_id) 6, Helper::strToInt(update_payload_1.substr(Macros::kTUPLE_ID_START, sizeof(tuple_id))));
  EXPECT_EQ((attribute_id) 8, Helper::strToInt(update_payload_1.substr(Macros::kAID_START, sizeof(attribute_id))));
  EXPECT_EQ(Macros::kIS_NUMBER + 0, update_payload_1.at(Macros::kIS_NUM_START));
  EXPECT_EQ(12, Helper::strToInt(update_payload_1.substr(Macros::kPRE_NUM_START, sizeof(int))));
  EXPECT_EQ(34, Helper::strToInt(update_payload_1.substr(Macros::kPOST_NUM_START, sizeof(int))));
  log_manager.forceToDisk("LOG");

  // String update
  std::string pre_str = "12";
  std::string post_str = "34";
  UpdateLogRecord update_log_record_2((TransactionId) 1, LogRecord::LogRecordType::kUPDATE, pre_str, post_str, (block_id) 1, (tuple_id) 2, (attribute_id) 3);
  log_manager.sendLogRequest(&update_log_record_2);
  log_manager.fetchNext();

  std::string update_payload_2 = log_manager.buffer_.substr(Macros::kHEADER_LENGTH);
  EXPECT_EQ((block_id) 1, Helper::strToId(update_payload_2.substr(Macros::kBID_START, sizeof(block_id))));
  EXPECT_EQ((tuple_id) 2, Helper::strToInt(update_payload_2.substr(Macros::kTUPLE_ID_START, sizeof(tuple_id))));
  EXPECT_EQ((attribute_id) 3, Helper::strToInt(update_payload_2.substr(Macros::kAID_START, sizeof(attribute_id))));
  EXPECT_EQ(Macros::kIS_STRING + 0, update_payload_2.at(Macros::kIS_NUM_START));
  EXPECT_EQ((int)pre_str.length(), Helper::strToInt(update_payload_2.substr(Macros::kPRE_NUM_START, sizeof(int))));
  EXPECT_EQ((int)post_str.length(), Helper::strToInt(update_payload_2.substr(Macros::kPOST_NUM_START, sizeof(int))));
  EXPECT_EQ(pre_str, update_payload_2.substr(Macros::kSTRING_START, pre_str.length()));
  EXPECT_EQ(post_str, update_payload_2.substr(Macros::kSTRING_START + pre_str.length(), post_str.length()));

}

} // namespace quickstep
