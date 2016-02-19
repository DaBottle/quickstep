#include "log/LogManager.hpp"
#include "transaction/Transaction.hpp"

#include "gtest/gtest.h"

namespace quickstep {

// Test the basic function of log table (check and update)
TEST(LogTableTest, EntryTest) {
  LogTable log_table;
  EXPECT_EQ(log_table.getPrevLSN((TransactionId) 1), (LSN) 0);
  // Add one entry
  log_table.update((TransactionId) 1, (LSN) 1);
  EXPECT_EQ(log_table.getPrevLSN((TransactionId) 1), (LSN) 1);
  EXPECT_EQ(log_table.getPrevLSN((TransactionId) 2), (LSN) 0);
  // Update the entry
  log_table.update((TransactionId) 1, (LSN) 5);
  EXPECT_EQ(log_table.getPrevLSN((TransactionId) 1), (LSN) 5);
  // Remove the entry
  log_table.remove((TransactionId) 1);
  EXPECT_EQ(log_table.getPrevLSN((TransactionId) 1), (LSN) 0);
}

}
