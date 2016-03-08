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
                  std::unordered_map<attribute_id, TypedValue>* old_value,
                  std::unordered_map<attribute_id, TypedValue>* updated_value)
  : LogRecord(tid, LogRecordType::kUPDATE)
  , bid_(bid)
  , tuple_id_(tupleId)
  , old_value_(old_value)
  , updated_value_(updated_value)  {}

  std::string UpdateLogRecord::payload() {
    std::string payload;
    // Location
    payload += Helper::idToStr(bid_) 
              + Helper::intToStr(tuple_id_);

    // Store attribute_id, pre/post value pair only when they are different
    for (std::unordered_map<attribute_id, TypedValue>::const_iterator update_it
            = updated_value_->begin();
         update_it != updated_value_->end();
         ++update_it) {
      // Attribute ID
      payload += Helper::intToStr(update_it->first);
      
      // pre type, length and value
      TypedValue pre_value = old_value_->at(update_it->first);
      std::uint8_t pre_type = pre_value.getTypeID()
                           | (pre_value.isNull() << Macros::kNULL_SHIFT) 
                           | (pre_value.ownsOutOfLineData() << Macros::kOWN_SHIFT);
      payload.append(1, (char) pre_type);
      if (!pre_value.isNull()) {
        std::uint8_t pre_length = pre_value.getDataSize() & 0xFF;
        payload.append(1, (char) pre_length);
        char* value = new char[(int) pre_length];
        pre_value.copyInto(value);
        payload += std::string(value);
      }

      TypedValue post_value = update_it->second;
      std::uint8_t post_type = post_value.getTypeID()
                            | (post_value.isNull() << Macros::kNULL_SHIFT) 
                            | (post_value.ownsOutOfLineData() << Macros::kOWN_SHIFT);
      payload.append(1, (char) post_type);
      if (!post_value.isNull()) {
        std::uint8_t post_length = post_value.getDataSize() & 0xFF;
        payload.append(1, (char) post_length);
        char* value = new char[(int) post_length];
        post_value.copyInto(value);
        payload += std::string(value);
      }
      //if (!EqualComparison::Instance().compareTypedValuesChecked(pre_value, pre_value.getTypeID(), post_value, post_value.getTypeID())) {

      //}
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
