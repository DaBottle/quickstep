#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "storage/StorageBlock.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "transaction/Transaction.hpp"
#include "types/containers/Tuple.hpp"
#include "gtest/gtest_prod.h"
#include <cstdint>
#include <string>

namespace quickstep {

using LSN = std::uint64_t;

class LogRecord {

public:
  /* Log Record Type */ 
  enum class LogRecordType : std::uint8_t {
    kEMPTY = 0,  // Used for unit test
    kUPDATE = 1,
    kCOMPENSATION = 2,
    kCOMMIT = 3,
    kABORT = 4,
    kCHECKPOINT = 5,
    kINSERT = 6,
    kDELETE = 7,
    kFORCE = 0xFF
  };

  LogRecord(TransactionId tid,
            LogRecordType log_record_type);

  TransactionId getTId();

  // Check the type
  bool isForce();

  bool isComplete();

  // Get the header of the log record (9 bytes: type(1) and tid(8))
  std::string header();

  // Get the payload of the log record
  virtual std::string payload();

protected:
  TransactionId tid_;  // Transaction ID number
  LogRecordType log_record_type_;

  FRIEND_TEST(LogManagerTest, HeaderTranslationTest);
};

/**
 * Update Log Record
 * 
 * Payload Format
 *   1. Number update (33 bytes)
 *     is_num(1) bid(8) tupleId(8) aid(8) pre_num(4) post_num(4)
 *   2. String update (33 bytes + 2 string length)
 *     is_num(1) bid(8) tupleId(8) aid(8) pre_num(4) pre_str(pre_num) post_num(4) post_str(post_num)
 */
class UpdateLogRecord : public LogRecord {
public:
  // Two constructor: one for string update, one for integer update
  UpdateLogRecord(TransactionId tid,
                  std::string pre_image,
                  std::string post_image,
                  block_id bid,
                  tuple_id tupleId,
                  attribute_id aid);

  UpdateLogRecord(TransactionId tid,
                  int pre_image,
                  int post_image,
                  block_id bid,
                  tuple_id tupleId,
                  attribute_id aid);

  virtual std::string payload() override;

private:
  bool isNum;
  // Strings will be empty if the update item is an integer
  std::string pre_str_;
  std::string post_str_;
  // Nums will be the length of string if the update item is a string
  int pre_num_;
  int post_num_;
  block_id bid_;
  tuple_id tuple_id_;
  attribute_id aid_;
};



/**
 * Commit Log Record
 */
class CommitLogRecord : public LogRecord {
public:
  CommitLogRecord(TransactionId tid);
};

/**
 * Abort Log Record
 */
class AbortLogRecord : public LogRecord {
public:
  AbortLogRecord(TransactionId tid);
};

class InsertLogRecord : public LogRecord {
public:
  InsertLogRecord(TransactionId tid,
                  LogRecordType log_record_type,
                  block_id bid,
                  Tuple* tuple);

private:
  block_id bid_;
  Tuple* tuple_;
};

} // namespace quickstep

#endif
