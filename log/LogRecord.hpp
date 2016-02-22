#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "storage/StorageBlock.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "transaction/Transaction.hpp"
#include "types/TypedValue.hpp"
#include "types/TypeID.hpp"
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
  UpdateLogRecord(TransactionId tid,
                  block_id bid,
                  tuple_id tupleId,
                  attribute_id aid,
                  TypedValue pre_typed_value,
                  TypedValue post_typed_value);

  // Return the payload of the log record
  virtual std::string payload() override;

private:
  block_id bid_;
  tuple_id tuple_id_;
  attribute_id aid_;
  // The lowest 6 bits is value type, the 7th is null flag, the 8th is out-of-line data ownership flag
  std::uint8_t pre_type_;
  // The length of out-of-line data
  std::uint8_t pre_length_;
  // The byte value string of data
  std::string pre_image_;
  std::uint8_t post_type_;
  std::uint8_t post_length_;
  std::string post_image_;
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
