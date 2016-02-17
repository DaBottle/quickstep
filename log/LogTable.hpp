#ifndef QUICKSTEP_LOG_LOG_TABLE_
#define QUICKSTEP_LOG_LOG_TABLE_

#include <unordered_map>
#include "transaction/Transaction.hpp"
#include "log/LogRecord.hpp"

namespace quickstep {

class LogTable {
public:
  // Constructor
  LogTable();

  // Return the previous LSN of given transaction, 0 if not exists
  LSN getPrevLSN(TransactionId tid);

  void updateTable(TransactionId tid, LSN prev_LSN);

private:
  std::unordered_map<TransactionId, LSN> log_table_;
};

} // namespace quickstep

#endif
