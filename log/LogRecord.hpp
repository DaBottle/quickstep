#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
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
  };

  LogRecord(TransactionId tid,
            LogRecordType log_record_type);

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

} // namespace quickstep

#endif
