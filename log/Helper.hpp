#ifndef QUICKSTEP_LOG_HELPER_
#define QUICKSTEP_LOG_HELPER_

#include <string>
#include <cstdint>
#include "types/TypedValue.hpp"

using namespace std;

namespace quickstep {

class Helper {
  
public:
  // Translate between id (64-bit) and string
  static string idToStr(uint64_t id);

  static uint64_t strToId(string str);

  // Translate between int and string
  static string intToStr(uint32_t num);

  static uint32_t strToInt(string str);

  // Translate TypedValue into string
  // Format: type(1): including type, isNull and ownsOutOfLineData
  //         length(1): the length of the value (only appears in Char and VarChar)
  //         value(length): value in bytes
  static string valueToStr(TypedValue value);

  static TypedValue strToValue(string);

  // get the length of the logged TypedValue
  static int valueLength(TypedValue value);

  // Judge if two TypedValue's equal to each other
  static bool valueEqual(TypedValue value1, TypedValue value2);
};

} // namespace quickstep

#endif
