#include "log/LogRecord.hpp"
#include "log/LogTable.hpp"
#include <unordered_map>

namespace quickstep {

  LogTable::LogTable()
    : log_table_() {}

  LSN LogTable::getPrevLSN(TransactionId tid) {
    std::unordered_map<TransactionId, LSN>::const_iterator it = log_table_.find(tid);
    if (it == log_table_.end()) {
      log_table_.insert(std::make_pair(tid, (LSN) 0));
      return 0;
    }
    else
      return it->second;
  }

} // namespace quickstep
