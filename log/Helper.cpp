#include "log/Helper.hpp"
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

} // namespace quickstep
