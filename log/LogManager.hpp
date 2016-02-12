#ifndef QUICKSTEP_LOG_LOG_MANAGER_
#define QUICKSTEP_LOG_LOG_MANAGER_

#include <string.h>
#include <atomic>
#include <cstdint>
#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"

namespace quickstep {

class LogManager {

public:
  LogManager();

  LSN getCurrentLSN(int length);

  //bool writeLog(LogRecord log_record);

  //bool flushToDisk();

private:
  std::atomic<LSN> current_LSN_; // 32 bits for file index, 32 bits for offsets in a log file
  LogTable log_table_;
};

} // namespace quickstep
#endif
