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

  void LogManager::logUpdate(TransactionId tid,
                             block_id bid,
                             tuple_id tupleId,
                             std::unordered_map<attribute_id, TypedValue>* old_value,
                             std::unordered_map<attribute_id, TypedValue>* updated_value) {
    UpdateLogRecord* record = new UpdateLogRecord(tid, bid, tupleId, old_value, updated_value);
    writeToBuffer(record);
  }

  void LogManager::sendForceRequest() {
    mutex_.lock();
    std::uint32_t log_index = current_LSN_ >> Macros::kLOG_INDEX_SHIFT;
    flushToDisk("LOG" + std::to_string(log_index));
    mutex_.unlock();
  }

  // Write log to buffer
  void LogManager::writeToBuffer(LogRecord* record) {
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

  // Write to disk
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

  void LogManager::printHeader() {
    int i;
    std::ofstream myFile ("header.test", std::ios::out | std::ios::trunc); 
    while (i < (int)buffer_.length()) {
      // Read header
      int length = Helper::strToInt(buffer_.substr(Macros::kLENGTH_START, Macros::kLENGTH_END));
      uint8_t type = buffer_.at(Macros::kTYPE_START);
      TransactionId tid = Helper::strToId(buffer_.substr(Macros::kTID_START, Macros::kTID_END));
      LSN current_LSN = Helper::strToId(buffer_.substr(Macros::kCURRENT_LSN_START, Macros::kCURRENT_LSN_END));
      LSN prev_LSN = Helper::strToId(buffer_.substr(Macros::kPREV_LSN_START, Macros::kPREV_LSN_END));
      LSN trans_prev_LSN = Helper::strToId(buffer_.substr(Macros::kTRANS_PREV_LSN_START, Macros::kTRANS_PREV_LSN_END));
      // Output header
      myFile << "length: " << length << '\n';
      myFile << "type: " << type << '\n';
      myFile << "Transaction ID: " << tid << '\n';
      myFile << "Current LSN: " << current_LSN << '\n';
      myFile << "previous LSN: " << prev_LSN << '\n';
      myFile << "previous LSN for this transaction: " << trans_prev_LSN << '\n';
      
      i += length;      
    } 
  }

} // namespace quickstep
