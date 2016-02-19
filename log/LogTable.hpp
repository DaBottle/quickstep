#ifndef QUICKSTEP_LOG_LOG_TABLE_
#define QUICKSTEP_LOG_LOG_TABLE_

#include <unordered_map>
#include "transaction/Transaction.hpp"
#include "log/LogRecord.hpp"
#include "gtest/gtest_prod.h"

namespace quickstep {

class LogTable {
public:
  // Constructor
  LogTable();

  // Return the previous LSN of given transaction, 0 if not exists
  LSN getPrevLSN(TransactionId tid);

  void update(TransactionId tid, LSN prev_LSN);

  void remove(TransactionId tid);

private:
  std::unordered_map<TransactionId, LSN> log_table_;
  FRIEND_TEST(LotTableTest, EntryTest);

  DISALLOW_COPY_AND_ASSIGN(LogTable);
};

} // namespace quickstep

#endif
