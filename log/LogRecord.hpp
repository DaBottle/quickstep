#ifndef QUICKSTEP_LOG_LOG_RECORD_
#define QUICKSTEP_LOG_LOG_RECORD_

#include "catalog/CatalogTypedefs.hpp"
#include "expressions/scalar/Scalar.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "storage/ValueAccessor.hpp"
#include "transaction/Transaction.hpp"
#include "types/TypedValue.hpp"
#include "types/TypeID.hpp"
#include "types/containers/Tuple.hpp"
#include "gtest/gtest_prod.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

namespace quickstep {

/** \addtogroup Log
 *  @{
 */

/**
 * @brief Base class for the log record
 **/
class LogRecord {

public:
  /**
   * Log types: currently supporting update, insert and delete
   **/
  enum LogRecordType {    
    kEMPTY = 0,  // Used for unit test
    kUPDATE = 1,
    kCOMPENSATION = 2,
    kCOMMIT = 3,
    kABORT = 4,
    kCHECKPOINT = 5,
    kINSERT = 6,
    kINSERT_BATCH = 7,
    kDELETE = 8,
    kREBUILD = 9,
  };

  /**
   * @brief Constructor.
   *
   * @param tid The id of the transaction that is writing the log.
   * @param log_record_type The type of the log.
   **/
  LogRecord(const TransactionId tid,
            const LogRecordType log_record_type);

  /**
   * @brief Get the transaction id from this log record.
   *
   * @return return the transaction id.
   **/
  TransactionId getTId() const;

  /**
   * @brief Judge if this log record means the end of the transaction.
   *
   * @return True if this means the transaction finishes (whether commit or
   *         abort, false otherwise.
   **/
  bool isComplete() const;

  /**
   * @brief Get the information needed to form the header.
   * @note The length of the return string is 9, where the first byte is the
   *       type of the log, and the next 8 bytes are the transaction id. The LSN
   *       part will be handled in the log manager.
   * 
   * @return A string contains the information needed for header.
   **/
  std::string header() const;

  /**
   * @brief Get the payload of the current log record.
   * @note The payload basically contains the location of the action, and the
   *       information needed for redo and undo the action.
   *
   * @return A string contains the formatted redo-undo information
   **/
  virtual std::string payload() const;

protected:
  const TransactionId tid_;
  const LogRecordType log_record_type_;
};

/**
 * Update Log Record
 */
class UpdateLogRecord : public LogRecord {
public:
  UpdateLogRecord(const TransactionId tid,
                  const block_id bid,
                  const tuple_id tupleId,
                  const std::unordered_map<attribute_id, TypedValue>* old_value,
                  const std::unordered_map<attribute_id, TypedValue>* updated_value);
                  
  // Format of update log payload:
  // First, bid(8) tuple_id(4)
  // Then for each attribute: attribute_id(4), 
  //                          pre_type(1), pre_length(1), pre_value(pre_length),
  //                          post_type(1), post_length(1), post_value(post_length)
  // For null value, only pre_type is written (no length and value)                              
  virtual std::string payload() const override;

private:
  const block_id bid_;
  const tuple_id tuple_id_;
  const std::unordered_map<attribute_id, TypedValue>* old_value_;
  const std::unordered_map<attribute_id, TypedValue>* updated_value_;
};

/**
 * Insert/Delete Log Record
 */
class InsertDeleteLogRecord : public LogRecord {
public:
  InsertDeleteLogRecord(const TransactionId tid,
                        const LogRecordType log_record_type,
                        const block_id bid,
                        const tuple_id tupleId,
                        const Tuple* tuple);

  // Format of insert/delete log payload:
  // bid(8) tuple_id(4)
  // For each attribute: pre_type(1), pre_length(1), pre_value(pre_length)
  // for null value, only pre_type is written
  virtual std::string payload() const override;

private:
  const block_id bid_;
  const tuple_id tuple_id_;
  const Tuple* tuple_;
};

/**
 * Rebuild Log Record
 **/
class RebuildLogRecord : public LogRecord {
public:
  RebuildLogRecord(const TransactionId tid,
                   const LogRecordType log_record_type,
                   const block_id bid,
                   ValueAccessor* accessor);

  // Format of rebuild log payload:
  // bid(8)
  // For each tuple: length(4), TypedValues
  virtual std::string payload() const override;

private:
  const block_id bid_;
  ValueAccessor* accessor_;
};

/**
 * Commit Log Record
 */
class CommitLogRecord : public LogRecord {
public:
  CommitLogRecord(const TransactionId tid);
};

/**
 * Abort Log Record
 */
class AbortLogRecord : public LogRecord {
public:
  AbortLogRecord(const TransactionId tid);
};

} // namespace quickstep

#endif
