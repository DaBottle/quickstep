#include "log/LogManager.hpp"

namespace quickstep {

  LogManager::LogManager() 
    : current_LSN_(1)
    , log_table_() {}

  LSN LogManager::getCurrentLSN(int length) {
    return current_LSN_.fetch_add(length);
  }

} // namespace quickstep
