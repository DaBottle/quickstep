#include "log/LogRecord.hpp"

namespace quickstep {

  LogRecord::LogRecord(TransactionId tid,
                       LogRecordType log_record_type)
    : tid_(tid)
    , log_record_type_(log_record_type)  {}

  UpdateLogRecord::UpdateLogRecord(TransactionId tid,
                          LogRecordType log_record_type,
                          std::string pre_image,
                          std::string post_image,
                          block_id bid,
                          tuple_id tupleId,
                          attribute_id aid)
    : LogRecord(tid, log_record_type)
    , pre_image_(pre_image)
    , post_image_(post_image)
    , bid_(bid)
    , tuple_id_(tupleId)
    , aid_(aid)  {}

} // namespace quickstep
