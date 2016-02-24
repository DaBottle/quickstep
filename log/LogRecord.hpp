#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "expressions/scalar/Scalar.hpp"
#include "storage/StorageBlock.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "transaction/Transaction.hpp"
#include "types/TypedValue.hpp"
#include "types/TypeID.hpp"
#include "types/containers/Tuple.hpp"
#include "gtest/gtest_prod.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

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
 *   bid(8) tupleId(4) aid(4) preType(1) preLength(1) preImage(varies) postType(1) postLength(1) postImage(varies)
 */
class UpdateLogRecord : public LogRecord {
public:
  UpdateLogRecord(TransactionId tid,
                  block_id pre_bid,
                  tuple_id pre_tuple_id,
                  block_id post_bid,
                  tuple_id post_tuple_id,
                  std::unordered_map<attribute_id, TypedValue>* pre_image,
                  std::unordered_map<attribute_id, TypedValue>* post_image);

  // Return the payload of the log record
  virtual std::string payload() override;

private:
  block_id pre_bid_;
  tuple_id pre_tuple_id_;
  block_id post_bid_;
  tuple_id post_tuple_id_;
  std::unordered_map<attribute_id, TypedValue>* pre_image_;
  std::unordered_map<attribute_id, TypedValue>* post_image_;
  /*
  // The lowest 6 bits is value type, the 7th is null flag, the 8th is out-of-line data ownership flag
  std::uint8_t pre_type_;
  // The length of out-of-line data
  std::uint8_t pre_length_;
  // The byte value string of data
  std::string pre_image_;
  std::uint8_t post_type_;
  std::uint8_t post_length_;
  std::string post_image_;
  */
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
