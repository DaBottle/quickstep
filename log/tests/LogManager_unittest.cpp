#include "log/Helper.hpp"
#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include "transaction/Transaction.hpp"
#include "types/containers/Tuple.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include "types/TypeFactory.hpp"
#include "types/TypeID.hpp"
#include <string.h>
#include <vector>

#include "gtest/gtest.h"

namespace quickstep {

// Test if LSN increments as expected
TEST(LogManagerTest, LSNTest) {
  LogManager log_manager;
  EXPECT_EQ(log_manager.current_LSN_, (LSN) 1);
  EXPECT_EQ(log_manager.prev_LSN_, (LSN) 0);

  // Add first record
  log_manager.logEmpty(TransactionId (1));
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH), log_manager.current_LSN_);
  EXPECT_EQ((LSN) 1, log_manager.prev_LSN_);

  // Add second record 
  log_manager.logEmpty(TransactionId (2));
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH * 2), log_manager.current_LSN_);
  EXPECT_EQ((LSN) (1 + Macros::kHEADER_LENGTH), log_manager.prev_LSN_);
}


// Test if buffer has the correct size (after writing and forcing to disk)
TEST(LogManagerTest, BufferSizeTest) {
  LogManager log_manager;
  EXPECT_EQ((int)log_manager.buffer_.length(), 0);

  // Write a record
  log_manager.logEmpty(TransactionId(1));
  EXPECT_EQ(Macros::kHEADER_LENGTH + 0, (int)log_manager.buffer_.length());

  // Force to disk
  log_manager.flushToDisk("LOG");
  EXPECT_EQ(0, (int)log_manager.buffer_.length());
}

// Test if the header is translated correctly
TEST(LogManagerTest, HeaderTranslationTest) {
  LogManager log_manager;
  // Record the record header
  std::uint64_t current_LSN_1 = log_manager.current_LSN_;
  std::uint64_t prev_LSN_1 = log_manager.prev_LSN_;
  std::uint64_t trans_prev_LSN_1 = log_manager.log_table_.getPrevLSN((TransactionId) 1);
  log_manager.logEmpty(TransactionId(1));
  // Check every fields' translation in the buffer
  std::string buffer = log_manager.buffer_;
  EXPECT_EQ(buffer.length(), Helper::strToInt(buffer.substr(Macros::kLENGTH_START, sizeof(int))));
  EXPECT_EQ((int)LogRecord::LogRecordType::kEMPTY, (int)buffer.at(Macros::kTYPE_START));
  EXPECT_EQ((TransactionId) 1, Helper::strToId(buffer.substr(Macros::kTID_START, sizeof(TransactionId))));
  EXPECT_EQ(current_LSN_1, Helper::strToId(buffer.substr(Macros::kCURRENT_LSN_START, sizeof(LSN))));
  EXPECT_EQ(prev_LSN_1, Helper::strToId(buffer.substr(Macros::kPREV_LSN_START, sizeof(LSN))));
  EXPECT_EQ(trans_prev_LSN_1, Helper::strToId(buffer.substr(Macros::kTRANS_PREV_LSN_START, sizeof(LSN))));
}

// Test if the update log record could be handled properly
TEST(LogManagerTest, UpdateTest) {
  LogManager log_manager;
  TransactionId tid(5);
  block_id bid(1);
  tuple_id tupleId(2);
  // Try values with different sizes (fixed and variable)
  // Integer (4 bytes)
  attribute_id aid_int(1);
  TypedValue old_int((int) 1);
  TypedValue new_int((int) 2);
  // Long (8 bytes)
  attribute_id aid_long(2);
  TypedValue old_long((long) 1);
  TypedValue new_long((long) 2);
  // String (variable sizes)
  attribute_id aid_nonnum(3);
  std::string str = "old string";
  TypedValue old_nonnum(kVarChar, str.c_str(), str.length());
  TypedValue new_nonnum(kVarChar);
  // Write log
  std::unordered_map<attribute_id, TypedValue> old_value = {{aid_int, old_int}, {aid_long, old_long}, {aid_nonnum, old_nonnum}};
  std::unordered_map<attribute_id, TypedValue> new_value = {{aid_int, new_int}, {aid_long, new_long}, {aid_nonnum, new_nonnum}};
  std::unordered_map<attribute_id, TypedValue>::iterator it;
  log_manager.logUpdate(tid, bid, tupleId, &old_value, &new_value);
  // Check translation
  // Check bid and tuple_id
  std::string payload = log_manager.buffer_.substr(Macros::kHEADER_LENGTH);
  int index = 0;
  EXPECT_EQ(bid, Helper::strToId(payload.substr(index, (int) sizeof(bid))));
  index += sizeof(bid);
  EXPECT_EQ(tupleId, (int) Helper::strToInt(payload.substr(index, (int) sizeof(tupleId))));
  index += sizeof(tupleId);
  // Check each attribute
  while (index < (int)payload.length()) {
    attribute_id aid = Helper::strToInt(payload.substr(index, sizeof(attribute_id)));
    index += sizeof(attribute_id);
    // Get the expected values
    it = old_value.find(aid);
    DEBUG_ASSERT(it != old_value.end());
    TypedValue pre_expect = it->second;
    it = new_value.find(aid);
    DEBUG_ASSERT(it != old_value.end());
    TypedValue post_expect = it->second;
    // Get the logged values
    TypedValue pre_log = Helper::strToValue(payload.substr(index));
    index += Helper::valueLength(pre_log);
    TypedValue post_log = Helper::strToValue(payload.substr(index));
    index += Helper::valueLength(post_log);
    // Compare
    if (!pre_log.isNull() || !pre_expect.isNull()) {
      EXPECT_TRUE(Helper::valueEqual(pre_log, pre_expect));
    }
    if (!post_log.isNull() || !post_expect.isNull()) {
      EXPECT_TRUE(Helper::valueEqual(post_log, post_expect));
    }
  }
}

// Test if insert/delete log record could be handled correctly
// Only test insert because delete is different only in header, which is tested in HeaderTranslationTest
TEST(LogManagerTest, InsertTest) {
  LogManager log_manager;
  TransactionId tid(1);  
  block_id bid(2);
  tuple_id tupleId(3);
  // Put different kind of TypedValue into a Tuple
  TypedValue int_value((int) 1);
  TypedValue long_value((long) 2);
  std::string str("insert string");
  TypedValue str_value(kVarChar, str.c_str(), str.length());
  TypedValue null_value(kVarChar);
  Tuple tuple({int_value, long_value, str_value, null_value});
  // Insert  
  log_manager.logInsert(tid, bid, tupleId, &tuple);
  // Check payload
  std::string payload = log_manager.buffer_.substr(Macros::kHEADER_LENGTH);
  // block_id and tuple_id  
  int index = 0;
  EXPECT_EQ(bid, Helper::strToId(payload.substr(index, sizeof(bid))));
  index += sizeof(bid);
  EXPECT_EQ(tupleId, (int)Helper::strToInt(payload.substr(index, sizeof(tupleId))));
  index += sizeof(tupleId);
  // Check each value in the tuple
  for (Tuple::const_iterator value_it = tuple.begin();
       value_it != tuple.end();
       value_it++) {
    TypedValue value_expect = *value_it;
    TypedValue value_log = Helper::strToValue(payload.substr(index));
    index += Helper::valueLength(value_log);
    if (!value_expect.isNull() || !value_log.isNull()) {  
      EXPECT_TRUE(Helper::valueEqual(value_expect, value_log));
    }
  }
  EXPECT_EQ((int)payload.length(), index);
}

/*
// Test if the log table would behave properly upon transaction commission and abortion
TEST(LogManagerTest, CommitAndAbortTest) {
  LogManager log_manager;
  TransactionId tid = 1;
  // Commit
  LogRecord empty(tid, LogRecord::LogRecordType::kEMPTY);
  CommitLogRecord commit_log_record(tid);
  log_manager.sendLogRequest(&empty);
  log_manager.sendLogRequest(&commit_log_record);
  log_manager.fetchNext();
  EXPECT_EQ((LSN) 1, log_manager.log_table_.getPrevLSN(tid));
  log_manager.fetchNext();
  EXPECT_EQ((LSN) 0, log_manager.log_table_.getPrevLSN(tid));

  // Abort
  AbortLogRecord abort_log_record(tid);
  log_manager.sendLogRequest(&empty);
  log_manager.sendLogRequest(&abort_log_record);
  log_manager.fetchNext();
  EXPECT_EQ((LSN) (1 + 2 * Macros::kHEADER_LENGTH), log_manager.log_table_.getPrevLSN(tid));
  log_manager.fetchNext();
  EXPECT_EQ((LSN) 0, log_manager.log_table_.getPrevLSN(tid));
}
*/
} // namespace quickstep
