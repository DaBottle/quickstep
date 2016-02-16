#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "storage/StorageBlock.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "transaction/Transaction.hpp"
#include <cstdint>
#include <string>

namespace quickstep {

using LSN = std::uint64_t;

class LogRecord {
public:
  /* Log Record Type */ 
  enum class LogRecordType : std::uint8_t {
    kUPDATE = 0,
    kCOMPENSATION = 1,
    kCOMMIT = 2,
    kABORT = 3,
    kCHECKPOINT = 4,
    kCOMPLETION = 5,
    kINSERT = 6,
    kDELETE = 7,
  };

  LogRecord(TransactionId tid,
            LogRecordType log_record_type);

  TransactionId getTId();

  // Translate id to string
  std::string idToStr(std::uint64_t id);

  // print the result to file
  virtual void print(FILE* log_file);

protected:
  TransactionId tid_;  // Transaction ID number
  LogRecordType log_record_type_;
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
