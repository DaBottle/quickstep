#include "log/Helper.hpp"
#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
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

  void LogManager::sendLogRequest(LogRecord* log_record) {
    in_queue_.push(log_record);
  }

  void LogManager::fetchNext() {
    // Fetch a new record
    LogRecord* log_record = in_queue_.popOne();
    // Check if it is a force request
    if (log_record->isForce()) {
      std::uint32_t force_index = current_LSN_ >> Macros::kLOG_INDEX_SHIFT;
      forceToDisk("LOG" + std::to_string(force_index));
      return;
    }

    std::string payload = log_record->payload();
    int length = payload.length() + Macros::kHEADER_LENGTH;

    // Add the LSN part to header
    // Three new fields:
    // current_LSN: the LSN of current log
    // prev_LSN: the LSN of previous log
    // trans_prev_LSN: the LSN of previous log for this transaction
    LSN prev_LSN = prev_LSN_;
    LSN trans_prev_LSN = log_table_.getPrevLSN(log_record->getTId());
    LSN current_LSN = current_LSN_.fetch_add(length);
    std::string LSNs = Helper::idToStr(current_LSN) + Helper::idToStr(prev_LSN) + Helper::idToStr(trans_prev_LSN);

    // Get the final string value of the record
    std::string record_str = Helper::intToStr(length) + log_record->header() + LSNs + payload;

    // Write phase: write to buffer
    std::uint32_t log_index = current_LSN >> Macros::kLOG_INDEX_SHIFT;
    std::uint32_t pre_log_index = prev_LSN_ >> Macros::kLOG_INDEX_SHIFT;
    // The current log will be on a new file. Before writing, flush the buffer first
    if (log_index != pre_log_index) {
      forceToDisk("LOG" + std::to_string(pre_log_index));
    }
    // write to buffer
    buffer_ += record_str;

    // Update private fields
    prev_LSN_ = current_LSN;
    ++record_count_;
    // Update log table
    if (log_record->isComplete()) {
      log_table_.remove(log_record->getTId());
    }
    else {
      log_table_.update(log_record->getTId(), current_LSN);
    }
  }

  void LogManager::sendForceRequest() {
    LogRecord force((TransactionId) 0, LogRecord::LogRecordType::kFORCE);
    in_queue_.push(&force);
    std::uint64_t target = record_count_ + in_queue_.size();
    while (record_count_ < target) {}
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
