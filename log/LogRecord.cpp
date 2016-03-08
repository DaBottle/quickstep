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
      
      // pre/post TypedValue
      payload += Helper::valueToStr(old_value_->at(update_it->first))
               + Helper::valueToStr(update_it->second);
    }
        
    return payload;
  }

  // InsertLogRecord
  InsertLogRecord::InsertLogRecord(TransactionId tid,
                                   block_id bid,
                                   tuple_id tupleId,
                                   Tuple* tuple)
  : LogRecord(tid, LogRecordType::kINSERT)
  , bid_(bid)
  , tuple_id_(tupleId)
  , tuple_(tuple) {}

  std::string InsertLogRecord::payload() {
    std::string payload;
    // block_id and tuple_id
    payload += Helper::idToStr(bid_) + Helper::intToStr(tuple_id_);
    // Tuple
    for (Tuple::const_iterator value_it = tuple_->begin();
         value_it != tuple_->end();
         value_it++) {
      payload += Helper::valueToStr(*value_it);
    }
    
    return payload;
  }

  // CommitLogRecord
  CommitLogRecord::CommitLogRecord(TransactionId tid)
    : LogRecord(tid, LogRecordType::kCOMMIT) {}

  // AbortLogRecord
  AbortLogRecord::AbortLogRecord(TransactionId tid)
    : LogRecord(tid, LogRecordType::kABORT) {}


} // namespace quickstep
