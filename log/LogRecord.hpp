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
    kSHIFT = 8,
  };

  LogRecord(TransactionId tid,
            LogRecordType log_record_type);

  TransactionId getTId();

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
 */
class UpdateLogRecord : public LogRecord {
public:
  UpdateLogRecord(TransactionId tid,
                  block_id bid,
                  tuple_id tupleId,
                  std::unordered_map<attribute_id, TypedValue>* old_value,
                  std::unordered_map<attribute_id, TypedValue>* updated_value);
                  
  // Format of update log payload:
  // First, bid(8) tuple_id(4)
  // Then for each attribute: attribute_id(4), 
  //                          pre_type(1), pre_length(1), pre_value(pre_length),
  //                          post_type(1), post_length(1), post_value(post_length)
  // For null value, only pre_type is written (no length and value)                              
  virtual std::string payload() override;

private:
  block_id bid_;
  tuple_id tuple_id_;
  std::unordered_map<attribute_id, TypedValue>* old_value_;
  std::unordered_map<attribute_id, TypedValue>* updated_value_;
};

/**
 * Insert/Delete Log Record
 */
class InsertDeleteLogRecord : public LogRecord {
public:
  InsertDeleteLogRecord(TransactionId tid,
                  LogRecordType log_record_type,
                  block_id bid,
                  tuple_id tupleId,
                  Tuple* tuple);

  // Format of insert/delete log payload:
  // bid(8) tuple_id(4)
  // For each attribute: pre_type(1), pre_length(1), pre_value(pre_length)
  // for null value, only pre_type is written
  virtual std::string payload() override;

private:
  block_id bid_;
  tuple_id tuple_id_;
  Tuple* tuple_;
};

/**
 * Shift Log Record
 */
class ShiftLogRecord : public LogRecord {
public:
  ShiftLogRecord(TransactionId tid,
                block_id bid,
                tuple_id pre_tuple_id,
                tuple_id post_tuple_id);

  virtual std::string payload() override;

private:
  block_id bid_;
  tuple_id pre_tuple_id_;
  tuple_id post_tuple_id_;
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

} // namespace quickstep

#endif
