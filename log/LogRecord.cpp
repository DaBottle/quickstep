#include "log/Helper.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include <string.h>
#include <utility>

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
                  block_id pre_bid,
                  tuple_id pre_tuple_id,
                  block_id post_bid,
                  tuple_id post_tuple_id,
                  std::unordered_map<attribute_id, TypedValue>* pre_image,
                  std::unordered_map<attribute_id, TypedValue>* post_image)
  : LogRecord(tid, LogRecordType::kUPDATE)
  , pre_bid_(pre_bid)
  , pre_tuple_id_(pre_tuple_id)
  , post_bid_(post_bid)
  , post_tuple_id_(post_tuple_id)
  , pre_image_(pre_image)
  , post_image_(post_image)  {}

  std::string UpdateLogRecord::payload() {
    std::string payload;
    // Location
    payload += Helper::idToStr(pre_bid_) 
              + Helper::intToStr(pre_tuple_id_) 
              + Helper::idToStr(post_bid_) 
              + Helper::intToStr(post_tuple_id_);
    // Action
    for (std::unordered_map<attribute_id, TypedValue>::const_iterator post_it
            = post_image_->begin();
         post_it != post_image_->end();
         ++post_it) {
      // Get the pre and post image, and write it to payload if there is a difference
      std::unordered_map<attribute_id, TypedValue>::const_iterator pre_it
          = pre_image_->find(post_it->first);
      if (pre_it == pre_image_->end()) {
        FATAL_ERROR("Unexpected result in UpdateLogRecord.payload(): Unknown attribute in post image");
      }
      int pre_length = (int) pre_it->second.getDataSize();
      int post_length = (int) post_it->second.getDataSize();
      char* pre_typed_value = new char[pre_length];
      char* post_typed_value = new char[post_length];
      pre_it->second.copyInto(pre_typed_value);
      post_it->second.copyInto(post_typed_value);
      int pre_type = pre_it->second.getTypeID() 
                   & (pre_it->second.isNull() << Macros::kNULL_SHIFT) 
                   & (pre_it->second.ownsOutOfLineData() << Macros::kOWN_SHIFT);
      int post_type = post_it->second.getTypeID() 
                   & (post_it->second.isNull() << Macros::kNULL_SHIFT) 
                   & (post_it->second.ownsOutOfLineData() << Macros::kOWN_SHIFT);
      std::string pre_str = std::string(1, (char)pre_length)
                          + std::string(1, (char)pre_type) 
                          + std::string(pre_typed_value);
      std::string post_str = std::string(1, (char)post_length) 
                           + std::string(1, (char)post_type) 
                           + std::string(post_typed_value);
      if (pre_str.compare(post_str) != 0) {
        payload += pre_str + post_str;
      }
    }
        
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
