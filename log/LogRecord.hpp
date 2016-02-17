#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "storage/StorageBlock.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "transaction/Transaction.hpp"
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
    kCOMPLETION = 6,
    kINSERT = 7,
    kDELETE = 8,
    kFORCE = 0xFF
  };

  LogRecord(TransactionId tid,
            LogRecordType log_record_type);

  TransactionId getTId();

  // Translate id to string
  std::string idToStr(std::uint64_t id);

  // Tell if it is a force request
  bool isForce();

  // Get the header of the log record (9 bytes: type(1) and tid(8))
  std::string header();

  // Get the payload of the log record
  virtual std::string payload();

protected:
  TransactionId tid_;  // Transaction ID number
  LogRecordType log_record_type_;

  FRIEND_TEST(LogManagerTest, HeaderTranslationTest);
};

class UpdateLogRecord : public LogRecord {
public:
  UpdateLogRecord(TransactionId tid,
                  LogRecordType log_record_type,
                  std::string pre_image,
                  std::string post_image,
                  block_id bid,
                  tuple_id tupleId,
                  attribute_id aid);

private:
  std::string pre_image_;
  std::string post_image_;
  block_id bid_;
  tuple_id tuple_id_;
  attribute_id aid_;
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
