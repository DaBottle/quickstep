#ifndef QUICKSTEP_LOG_HELPER_
#define QUICKSTEP_LOG_HELPER_

#include <string>
#include <cstdint>
#include "types/TypedValue.hpp"

namespace quickstep {

class Helper {
  
public:
  // Translate between id (64-bit) and string
  static std::string idToStr(std::uint64_t id);

  static std::uint64_t strToId(std::string str);

  // Translate between int and string
  static std::string intToStr(int num);

  static int strToInt(std::string str);

  // Translate TypedValue into string
  // Format: type(1): including type, isNull and ownsOutOfLineData
  //         length(1): the length of the value
  //         value(length): value in bytes
  static std::string valueToStr(TypedValue value);
};

} // namespace quickstep

#endif
