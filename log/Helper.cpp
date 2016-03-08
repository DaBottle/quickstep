#include "log/Helper.hpp"
#include "log/Macros.hpp"
#include <string.h>
#include <cstdint>

namespace quickstep {

  std::string Helper::idToStr(std::uint64_t id) {
    std::string str;
    for (int i = 0; i < 8; ++i) { 
      str = (char)(id & 0xFF) + str;
      id >>= 8;
    }
    return str;
  }

  std::string Helper::intToStr(int num) {
    std::string str;
    for (int i = 0; i < 4; ++i) { 
      str = (char)(num & 0xFF) + str;
      num >>= 8;
    }
    return str;
  }

  int Helper::strToInt(std::string str) {
    int num;
    for (int i = 0; i < 4; ++i) {
      num <<= 8;
      num += str.at(i);
    }
    return num;
  }

  std::uint64_t Helper::strToId(std::string str) {
    std::uint64_t id;
    for (int i = 0; i < 8; ++i) {
      id <<= 8;
      id += str.at(i);
    }
    return id;
  }

  std::string Helper::valueToStr(TypedValue value) {
    std::string str;
    std::uint8_t type = value.getTypeID()
                      | (value.isNull() << Macros::kNULL_SHIFT) 
                      | (value.ownsOutOfLineData() << Macros::kOWN_SHIFT);
    str.append(1, (char) type);
    if (!value.isNull()) {
      std::uint8_t length = value.getDataSize();
      str.append(1, (char) length);
      char* value_copy = new char[length];
      value.copyInto(value_copy);
      str += std::string(value_copy, length);
    }
    
    return str;
  }

} // namespace quickstep
