#include "log/LogRecord.hpp"
#include <string.h>

namespace quickstep {
  // Constructors
  LogRecord::LogRecord(TransactionId tid,
                       LogRecordType log_record_type)
    : tid_(tid)
    , log_record_type_(log_record_type)  {}

  UpdateLogRecord::UpdateLogRecord(TransactionId tid,
                          LogRecordType log_record_type,
                          std::string pre_image,
                          std::string post_image,
                          block_id bid,
                          tuple_id tupleId,
                          attribute_id aid)
    : LogRecord(tid, log_record_type)
    , pre_image_(pre_image)
    , post_image_(post_image)
    , bid_(bid)
    , tuple_id_(tupleId)
    , aid_(aid)  {}

  InsertLogRecord::InsertLogRecord(TransactionId tid,
                                   LogRecordType log_record_type,
                                   block_id bid,
                                   Tuple* tuple)
    : LogRecord(tid, log_record_type)
    , bid_(bid)
    , tuple_(tuple)  {}

  // Return private fields
  TransactionId LogRecord::getTId() {
    return tid_;
  }

  // Translate id to string
  std::string LogRecord::idToStr(std::uint64_t id) {
    std::string str = "";
    for (int i = 0; i < 8; ++i) { 
      str = (char)(id & 0xFF) + str;
      id >>= 8;
    }
    return str;
  }

  bool LogRecord::isForce() {
    return log_record_type_ == LogRecordType::kFORCE;
  }

  // Print methods
  std::string LogRecord::header() {
    std::string header;
    header += (char)log_record_type_ + idToStr(tid_);
    return header;
  }

  std::string LogRecord::payload() {
    return "";
  }

} // namespace quickstep
