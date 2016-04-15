#include "log/Helper.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include "storage/ValueAccessor.hpp"
#include "storage/ValueAccessorUtil.hpp"
#include "types/containers/Tuple.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include <string.h>
#include <utility>

// (TODO)1.Partition 2.lock 3.index 4.Jignesh 5.str->char* 6.fsync 
namespace quickstep {
  // LogRecord
  LogRecord::LogRecord(const TransactionId tid,
                       const LogRecordType log_record_type)
    : tid_(tid)
    , log_record_type_(log_record_type)  {}

  TransactionId LogRecord::getTId() const {
    return tid_;
  }

  bool LogRecord::isComplete() const {
    return log_record_type_ == LogRecordType::kCOMMIT || log_record_type_ == LogRecordType::kABORT;
  }

  // Print methods
  std::string LogRecord::header() const {
    std::string header;
    header += (char)log_record_type_;
    header += Helper::idToStr(tid_);
    return header;
  }

  std::string LogRecord::payload() const {
    return "";
  }

  // UpdateLogRecord
  UpdateLogRecord::UpdateLogRecord(const TransactionId tid,
                                  const block_id bid,
                                  const tuple_id tupleId,
                                  const std::unordered_map<attribute_id, TypedValue>* old_value,
                                  const std::unordered_map<attribute_id, TypedValue>* updated_value)
  : LogRecord(tid, LogRecordType::kUPDATE)
  , bid_(bid)
  , tuple_id_(tupleId)
  , old_value_(old_value)
  , updated_value_(updated_value)  {}

  std::string UpdateLogRecord::payload() const {
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

  // InsertDeleteLogRecord
  InsertDeleteLogRecord::InsertDeleteLogRecord(const TransactionId tid,
                                              const LogRecordType log_record_type,
                                              const block_id bid,
                                              const tuple_id tupleId,
                                              const Tuple* tuple)
  : LogRecord(tid, log_record_type)
  , bid_(bid)
  , tuple_id_(tupleId)
  , tuple_(tuple) {}

  std::string InsertDeleteLogRecord::payload() const {
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

  // RebuildLogRecord
  RebuildLogRecord::RebuildLogRecord(const TransactionId tid,
                                     const LogRecordType log_record_type,
                                     const block_id bid,
                                     ValueAccessor* accessor)
  : LogRecord(tid, log_record_type)
  , bid_(bid)
  , accessor_(accessor) {}

  std::string RebuildLogRecord::payload() const {
    return InvokeOnAnyValueAccessor (accessor_, [&] (auto *accessor_)
        -> std::string {
      std::string payload;
      payload += Helper::idToStr(bid_);
      accessor_->beginIteration();
      while (accessor_->next()) {
        int length;
        std::string current_tuple_str;
        Tuple* tuple = accessor_->getTuple();
        for (Tuple::const_iterator tuple_it = tuple->begin();
           tuple_it != tuple->end();
           tuple_it++) {
          current_tuple_str += Helper::valueToStr(*tuple_it);
        }
        length = current_tuple_str.length();
        payload += Helper::intToStr(length) + current_tuple_str;
      }

      return payload;
    });
  }

  // CommitLogRecord
  CommitLogRecord::CommitLogRecord(const TransactionId tid)
    : LogRecord(tid, LogRecordType::kCOMMIT) {}

  // AbortLogRecord
  AbortLogRecord::AbortLogRecord(const TransactionId tid)
    : LogRecord(tid, LogRecordType::kABORT) {}


} // namespace quickstep
