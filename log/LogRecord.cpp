#include "log/Helper.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
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
    , isNum(false)
    , pre_str_(pre_image)
    , post_str_(post_image)
    , pre_num_((int)pre_image.length())
    , post_num_((int)post_image.length())
    , bid_(bid)
    , tuple_id_(tupleId)
    , aid_(aid)  {}

  UpdateLogRecord::UpdateLogRecord(TransactionId tid,
                          LogRecordType log_record_type,
                          int pre_image,
                          int post_image,
                          block_id bid,
                          tuple_id tupleId,
                          attribute_id aid)
    : LogRecord(tid, log_record_type)
    , isNum(true)
    , pre_str_()
    , post_str_()
    , pre_num_(pre_image)
    , post_num_(post_image)
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

  bool LogRecord::isForce() {
    return log_record_type_ == LogRecordType::kFORCE;
  }

  // Print methods
  std::string LogRecord::header() {
    std::string header;
    header += (char)log_record_type_ + Helper::idToStr(tid_);
    return header;
  }

  std::string LogRecord::payload() {
    return "";
  }

  std::string UpdateLogRecord::payload() {
    std::string payload;
    payload += Helper::idToStr(bid_) + Helper::intToStr(tuple_id_) + Helper::intToStr(aid_);
    if (isNum) {
      payload += (char) Macros::kIS_NUMBER + Helper::intToStr(pre_num_) + Helper::intToStr(post_num_);
    }
    else {
      payload += (char) Macros::kIS_STRING + Helper::intToStr(pre_num_) + Helper::intToStr(post_num_) + pre_str_ + post_str_;
    }
    
    return payload;
  }

} // namespace quickstep
