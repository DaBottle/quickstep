#include "log/Helper.hpp"
#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include "utility/ThreadSafeQueue.hpp"
#include <utility>
#include <string.h>
#include <iostream>
#include <fstream>
#include <mutex>

namespace quickstep {
  LogManager::LogManager() 
    : current_LSN_(1)
    , prev_LSN_(0)
    , log_table_() 
    , buffer_()
    , mutex_()  {}

  // Logging API
  void LogManager::logUpdate(const TransactionId tid,
                             const block_id bid,
                             const tuple_id tupleId,
                             const std::unordered_map<attribute_id, TypedValue>* old_value,
                             const std::unordered_map<attribute_id, TypedValue>* updated_value) {
    UpdateLogRecord* record = new UpdateLogRecord(tid, bid, tupleId, old_value, updated_value);
    writeToBuffer(record);
  }

  void LogManager::logInsert(const TransactionId tid,
                             const block_id bid,
                             const tuple_id tupleId,
                             const Tuple* tuple) {
    InsertDeleteLogRecord* record = new InsertDeleteLogRecord(tid, LogRecord::LogRecordType::kINSERT, bid, tupleId, tuple);
    writeToBuffer(record);
  }

  void LogManager::logInsertInBatch(const TransactionId tid,
                                    const block_id bid,
                                    const tuple_id tupleId,
                                    const Tuple* tuple) {
    InsertDeleteLogRecord* record = new InsertDeleteLogRecord(tid, LogRecord::LogRecordType::kINSERT_BATCH, bid, tupleId, tuple);
    writeToBuffer(record);
  }

  void LogManager::logDelete(const TransactionId tid,
                             const block_id bid,
                             const tuple_id tupleId,
                             const Tuple* tuple) {
    InsertDeleteLogRecord* record = new InsertDeleteLogRecord(tid, LogRecord::LogRecordType::kDELETE, bid, tupleId, tuple);
    writeToBuffer(record);
  }

  void LogManager::sendForceRequest() {
    mutex_.lock();
    std::uint32_t log_index = current_LSN_ >> Macros::kLOG_INDEX_SHIFT;
    flushToDisk("LOG" + std::to_string(log_index));
    mutex_.unlock();
  }

  std::string LogManager::getBuffer() {
    return buffer_;
  }

  LSN LogManager::getCurrentLSN() {
    return current_LSN_;
  }

  LSN LogManager::getPrevLSN() {
    return prev_LSN_;
  }

  LSN LogManager::getTransPrevLSN(TransactionId tid) {
    return log_table_.getPrevLSN(tid);
  }    

  // Write log to buffer
  void LogManager::writeToBuffer(const LogRecord* record) {
    std::string payload = record->payload();
    int length = payload.length() + Macros::kHEADER_LENGTH;

    // Add the LSN part to header (crictical session regarding to LSN and buffer)
    // Three new fields:
    // current_LSN: the LSN of current log
    // prev_LSN: the LSN of previous log
    // trans_prev_LSN: the LSN of previous log for this transaction
    mutex_.lock();
    LSN prev_LSN = prev_LSN_;
    LSN trans_prev_LSN = log_table_.getPrevLSN(record->getTId());
    std::string LSNs = Helper::idToStr(current_LSN_) + Helper::idToStr(prev_LSN) + Helper::idToStr(trans_prev_LSN);

    // Get the final string value of the record
    std::string record_str = Helper::intToStr(length) + record->header() + LSNs + payload;

    // Write phase: write to buffer
    std::uint32_t log_index = current_LSN_ >> Macros::kLOG_INDEX_SHIFT;
    std::uint32_t pre_log_index = prev_LSN_ >> Macros::kLOG_INDEX_SHIFT;
    // The current log will be on a new file. Before writing, flush the buffer first
    if (log_index != pre_log_index) {
      flushToDisk("LOG" + std::to_string(pre_log_index));
    }
    // write to buffer
    buffer_ += record_str;

    // Update private fields
    prev_LSN_ = current_LSN_;
    current_LSN_ += length;

    // Update log table
    if (record->isComplete()) {
      log_table_.remove(record->getTId());
    }
    else {
      log_table_.update(record->getTId(), prev_LSN);
    }
    mutex_.unlock();  // End of critical session
  }

  // Write to disk: fsync()
  void LogManager::flushToDisk(std::string filename) {
    std::ofstream myFile;
    myFile.open(filename, std::ios::out | std::ios::app);
    if (!myFile.is_open())  {
      FATAL_ERROR("Unexpected result in LogManger.flushToDisk: "
                  "Cannot open file " + filename);
    }
    myFile << buffer_;
    buffer_.clear();
    myFile.close();
  }

  void LogManager::logEmpty(TransactionId tid) {
    LogRecord* record = new LogRecord(tid, LogRecord::LogRecordType::kEMPTY);
    writeToBuffer(record);
  }

} // namespace quickstep
