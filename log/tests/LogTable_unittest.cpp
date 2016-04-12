#include "log/LogManager.hpp"
#include "transaction/Transaction.hpp"

#include "gtest/gtest.h"

namespace quickstep {

// Test the basic function of log table (check and update)
TEST(LogTableTest, EntryTest) {
  LogTable log_table;
  LSN empty = 0;
  TransactionId tid = 1;
  
  EXPECT_EQ(empty, log_table.getPrevLSN(tid));
  // Add one entry
  LSN input = 1;
  log_table.update(tid, input);
  EXPECT_EQ(input, log_table.getPrevLSN(tid));
  EXPECT_EQ(empty, log_table.getPrevLSN(tid + 1));
  // Update the entry
  input = 3;
  log_table.update(tid, input);
  EXPECT_EQ(input, log_table.getPrevLSN(tid));
  // Remove the entry
  log_table.remove(tid);
  EXPECT_EQ(empty, log_table.getPrevLSN(tid));
}

}
