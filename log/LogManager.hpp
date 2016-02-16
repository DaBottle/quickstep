#ifndef QUICKSTEP_LOG_LOG_MANAGER_
#define QUICKSTEP_LOG_LOG_MANAGER_

#include <string.h>
#include <atomic>
#include <cstdint>
#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"

namespace quickstep {

#define FILE_INDEX_SHIFT 32
#define OFFSET_MASK 0x0000FFFF

class LogManager {

public:
  LogManager();

  // Write the given log to the log file
  bool writeLog(LogRecord log_record);

  // Translate LSN to string in bit-style for file writing
  std::string LSNToStr(LSN current_LSN);

  //bool flushToDisk();

private:
  std::atomic<LSN> current_LSN_; // 32 bits for file index, 32 bits for offsets in a log file
  LSN prev_LSN_;
  LogTable log_table_;
};

} // namespace quickstep
#endif
