#include "log/Helper.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include <string.h>

namespace quickstep {
  // LogRecord
  LogRecord::LogRecord(TransactionId tid,
                       LogRecordType log_record_type)
    : tid_(tid)
    , log_record_type_(log_record_type)  {}

  TransactionId LogRecord::getTId() {
    return tid_;
  }

  bool LogRecord::isForce() {
    return log_record_type_ == LogRecordType::kFORCE;
  }

  bool LogRecord::isComplete() {
    return log_record_type_ == LogRecordType::kCOMMIT || log_record_type_ == LogRecordType::kABORT;
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

  // UpdateLogRecord
  UpdateLogRecord::UpdateLogRecord(TransactionId tid,
                  block_id bid,
                  tuple_id tupleId,
                  attribute_id aid,
                  TypedValue pre_typed_value,
                  TypedValue post_typed_value)
  : LogRecord(tid, LogRecordType::kUPDATE)
  , bid_(bid)
  , tuple_id_(tupleId)
  , aid_(aid)
  , pre_type_(pre_typed_value.getTypeID() 
            | pre_typed_value.isNull() << Macros::kNULL_SHIFT
            | pre_typed_value.ownsOutOfLineData() << Macros::kOWN_SHIFT)
  , pre_length_(pre_typed_value.getDataSize() & 0xFF)
  , pre_image_()
  , post_type_(post_typed_value.getTypeID() 
            | post_typed_value.isNull() << Macros::kNULL_SHIFT
            | post_typed_value.ownsOutOfLineData() << Macros::kOWN_SHIFT)
  , post_length_(post_typed_value.getDataSize() & 0xFF)
  , post_image_()
  {
    // Only having image if it is not null
    if (!pre_typed_value.isNull()) {
      char* pre_buffer = new char[(int)pre_length_ + 1];
      pre_typed_value.copyInto(pre_buffer);
      pre_image_ = std::string(pre_buffer);
    }
    if (!post_typed_value.isNull()) {
      char* post_buffer = new char[(int)post_length_ + 1];
      post_typed_value.copyInto(post_buffer);
      post_image_ = std::string(post_buffer);
    }
  }

  std::string UpdateLogRecord::payload() {
    std::string payload;
    // Location
    payload += Helper::idToStr(bid_) 
              + Helper::intToStr(tuple_id_) 
              + Helper::intToStr(aid_);
    // Action
    payload += (char) pre_type_;
    payload += (char) pre_length_;
    payload += pre_image_;
    payload += (char) post_type_;
    payload += (char) post_length_;
    payload += post_image_;

    return payload;
  }

  // CommitLogRecord
  CommitLogRecord::CommitLogRecord(TransactionId tid)
    : LogRecord(tid, LogRecordType::kCOMMIT) {}

  // AbortLogRecord
  AbortLogRecord::AbortLogRecord(TransactionId tid)
    : LogRecord(tid, LogRecordType::kABORT) {}

  InsertLogRecord::InsertLogRecord(TransactionId tid,
                                   LogRecordType log_record_type,
                                   block_id bid,
                                   Tuple* tuple)
    : LogRecord(tid, log_record_type)
    , bid_(bid)
    , tuple_(tuple)  {}

} // namespace quickstep
