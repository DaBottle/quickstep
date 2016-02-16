#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include <string.h>
#include <stdio.h>

namespace quickstep {
  LogManager::LogManager() 
    : current_LSN_(1)
    , log_table_() {}

  //TODO(Shixuan): translate file name, fseek to write
  bool LogManager::writeLog(LogRecord log_record) {
    // Update the LSN information in the log record and log manager
    // Three new fields:
    // current_LSN: the LSN of current log
    // prev_LSN: the LSN of previous log
    // trans_prev_LSN: the LSN of previous log for this transaction
    LSN prev_LSN = prev_LSN_;
    LSN trans_prev_LSN = log_table_.getPrevLSN(log_record.getTId());
    int length = sizeof(log_record) + sizeof(int) + 3 * sizeof(LSN); // one log record, 3 LSNs and one length
    LSN current_LSN = current_LSN_.fetch_add(length);
    std::string LSNs = LSNToStr(current_LSN) + LSNToStr(prev_LSN) + LSNToStr(trans_prev_LSN);
    prev_LSN_ = current_LSN;

    // Write phase: write to the specific position defined by LSN
    std::uint32_t log_index = current_LSN >> FILE_INDEX_SHIFT;
    long int offset = current_LSN & OFFSET_MASK;
    std::string filename_str = "LOG" + std::to_string(log_index);
    const char* filename = filename_str.c_str();
    std::FILE* log_file = fopen(filename, "a+");
    fseek(log_file, offset, SEEK_SET); // CONCERN: the offset is beyond SEEK_END (to be checked)
    fprintf(log_file, "%s", LSNs.c_str());

    return true;
  }

  std::string LogManager::LSNToStr(LSN current_LSN) {
    std::string str = "";
    for (int i = 0; i < 8; ++i) { 
      str = (char)(current_LSN & 0xFF) + str;
      current_LSN >>= 8;
    }
    return str;
  }

} // namespace quickstep
