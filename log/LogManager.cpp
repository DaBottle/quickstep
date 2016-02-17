#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "utility/ThreadSafeQueue.hpp"
#include <utility>
#include <string.h>
#include <iostream>
#include <fstream>

namespace quickstep {
  LogManager::LogManager() 
    : current_LSN_(1)
    , prev_LSN_(0)
    , log_table_() 
    , in_queue_()
    , record_count_(0)
    , buffer_()  {}

  void LogManager::sendLogRequest(LogRecord log_record) {
    in_queue_.push(log_record);
  }

  void LogManager::fetchNext() {
    // Fetch a new record
    LogRecord log_record = in_queue_.popOne();
    // Check if it is a force request
    if (log_record.isForce()) {
      std::uint32_t force_index = current_LSN_ >> LOG_INDEX_SHIFT;
      forceToDisk("LOG" + std::to_string(force_index));
      return;
    }

    std::string payload = log_record.payload();
    int length = payload.length() + HEADER_LENGTH;

    // Add the LSN part to header
    // Three new fields:
    // current_LSN: the LSN of current log
    // prev_LSN: the LSN of previous log
    // trans_prev_LSN: the LSN of previous log for this transaction
    LSN prev_LSN = prev_LSN_;
    LSN trans_prev_LSN = log_table_.getPrevLSN(log_record.getTId());
    LSN current_LSN = current_LSN_.fetch_add(length);
    std::string LSNs = idToStr(current_LSN) + idToStr(prev_LSN) + idToStr(trans_prev_LSN);

    // Get the final string value of the record
    std::string record_str = intToStr(length) + log_record.header() + LSNs;

    // Write phase: write to buffer
    std::uint32_t log_index = current_LSN >> LOG_INDEX_SHIFT;
    std::uint32_t pre_log_index = prev_LSN_ >> LOG_INDEX_SHIFT;
    // The current log will be on a new file. Before writing, flush the buffer first
    if (log_index != pre_log_index) {
      forceToDisk("LOG" + std::to_string(pre_log_index));
    }
    // write to buffer
    buffer_ += record_str;
    // Update private fields
    prev_LSN_ = current_LSN;
    log_table_.updateTable(log_record.getTId(), current_LSN);
    ++record_count_;
  }

  void LogManager::sendForceRequest() {
    LogRecord force((TransactionId) 0, LogRecord::LogRecordType::kFORCE);
    in_queue_.push(force);
    std::uint64_t target = record_count_ + in_queue_.size();
    while (record_count_ < target) {}
  }

  // Translations between number and string value (bit-style)
  std::string LogManager::idToStr(std::uint64_t id) {
    std::string str;
    for (int i = 0; i < 8; ++i) { 
      str = (char)(id & 0xFF) + str;
      id >>= 8;
    }
    return str;
  }

  std::string LogManager::intToStr(int num) {
    std::string str;
    for (int i = 0; i < 4; ++i) { 
      str = (char)(num & 0xFF) + str;
      num >>= 8;
    }
    return str;
  }

  int LogManager::strToInt(std::string str) {
    int num;
    for (int i = 0; i < 4; ++i) {
      num <<= 8;
      num += str.at(i);
    }
    return num;
  }

  std::uint64_t LogManager::strToId(std::string str) {
    std::uint64_t id;
    for (int i = 0; i < 8; ++i) {
      id <<= 8;
      id += str.at(i);
    }
    return id;
  }

  // Write to disk
  void LogManager::forceToDisk(std::string filename) {
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

  void LogManager::printHeader() {
    int i;
    std::ofstream myFile ("header.test", std::ios::out | std::ios::trunc); 
    while (i < (int)buffer_.length()) {
      // Read header
      int length = strToInt(buffer_.substr(LENGTH_START, LENGTH_END));
      uint8_t type = buffer_.at(TYPE_START);
      TransactionId tid = strToId(buffer_.substr(TID_START, TID_END));
      LSN current_LSN = strToId(buffer_.substr(CURRENT_LSN_START, CURRENT_LSN_END));
      LSN prev_LSN = strToId(buffer_.substr(PREV_LSN_START, PREV_LSN_END));
      LSN trans_prev_LSN = strToId(buffer_.substr(TRANS_PREV_LSN_START, TRANS_PREV_LSN_END));
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
