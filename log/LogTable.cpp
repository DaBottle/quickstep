#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"
#include <unordered_map>

namespace quickstep {

  LogTable::LogTable()
    : log_table_() {}

  LSN LogTable::getPrevLSN(TransactionId tid) {
    std::unordered_map<TransactionId, LSN>::const_iterator it = log_table_.find(tid);
    if (it == log_table_.end()) {
      return 0;
    }
    else
      return it->second;
  }

  void LogTable::updateTable(TransactionId tid, LSN prev_LSN) {
    log_table_[tid] = prev_LSN;
  }

} // namespace quickstep
