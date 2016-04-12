#ifndef QUICKSTEP_LOG_LOG_TABLE_
#define QUICKSTEP_LOG_LOG_TABLE_

#include <unordered_map>
#include "transaction/Transaction.hpp"
#include "gtest/gtest_prod.h"

namespace quickstep {

typedef std::uint64_t LSN;

/** \addtogroup Log
 *  @{
 */

/**
 * @brief A hash table to store the previous LSN for each running transaction.
 **/
class LogTable {
public:
  /**
   * @brief Constuctor.
   **/ 
  LogTable();

  
  /**
   * @brief Get the previous LSN for the given transaction.
   *
   * @param tid The id of the transaction to look up.
   *
   * @return The previous LSN of the given transaction, 0 if the transaction
   *         has no previous LSN.
   **/ 
  LSN getPrevLSN(const TransactionId tid);

  /**
   * @brief Update entry of the given transaction to a new value.
   *
   * @param tid The id of the transaction to update.
   * @param prev_LSN The new previous LSN for the given transaction.
   **/
  void update(const TransactionId tid, const LSN prev_LSN);

  /**
   * @brief Remove the given transaction from the table
   *
   * @param tid The id of the transaction to remove
   **/
  void remove(const TransactionId tid);

private:
  std::unordered_map<TransactionId, LSN> log_table_;

  DISALLOW_COPY_AND_ASSIGN(LogTable);
};

} // namespace quickstep

#endif
