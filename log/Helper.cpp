#include "log/Helper.hpp"
#include "log/Macros.hpp"
#include "types/DatetimeLit.hpp"
#include "types/IntervalLit.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include "types/TypeFactory.hpp"
#include "types/TypeID.hpp"
#include <string.h>
#include <cstdint>
#include <iostream>

namespace quickstep {

  std::string Helper::idToStr(std::uint64_t id) {
    return std::string(reinterpret_cast<char*>(&id), sizeof(id));
  }

  std::string Helper::intToStr(std::uint32_t num) {
    return std::string(reinterpret_cast<char*>(&num), sizeof(num));
  }

  std::uint32_t Helper::strToInt(std::string str) {
    return *reinterpret_cast<const std::uint32_t*>(str.c_str());
  }

  std::uint64_t Helper::strToId(std::string str) {
    return *reinterpret_cast<const std::uint64_t*>(str.c_str());
  }

  std::string Helper::valueToStr(TypedValue value) {
    std::string str;
    std::uint8_t info = value.getTypeID()
                      | (value.isNull() << Macros::kNULL_SHIFT) 
                      | (value.ownsOutOfLineData() << Macros::kOWN_SHIFT);
    info += 0;
    str.append(1, (char) info);
    if (!value.isNull()) {
      std::uint64_t length = value.getDataSize();
      if (value.getTypeID() == kChar || value.getTypeID() == kVarChar) {
        str += Helper::idToStr(length);
      }
      char* value_copy = new char[length];
      value.copyInto(value_copy);
      str += std::string(value_copy, length);
    }
    return str;
  }

  TypedValue Helper::strToValue(std::string str) {
    std::uint8_t info = str.at(0);
    TypeID type = static_cast<TypeID>(info & Macros::kTYPE_MASK);
    bool isNull = (info >> Macros::kNULL_SHIFT) & 0x01;
    bool owns = (info >> Macros::kOWN_SHIFT) & 0x01;
    // Null value
    if (isNull) {
      return TypedValue(type);
    }
    // Non-numeric data
    if (type == kChar || type == kVarChar) {
      std::uint64_t length = Helper::strToId(str.substr(1, 8));
      char* value_ptr;
      if (owns) {
        value_ptr = (char*) malloc(length);
        strcpy(value_ptr, str.substr(9, length).c_str());
        return TypedValue::CreateWithOwnedData(type, value_ptr, length);
      }
      else {
        char* data = new char[length];
        strcpy(data, str.substr(9, length).c_str());
        return TypedValue(type, data, length);
      }
    }
    // Numeric data
    std::uint32_t val_32;
    std::uint64_t val_64;
    switch (type) {
      case kInt:
        val_32 = Helper::strToInt(str.substr(1, sizeof(int)));
        return TypedValue(*reinterpret_cast<int*>(&val_32));
      case kFloat:
        val_32 = Helper::strToInt(str.substr(1, sizeof(int)));
        return TypedValue((float)*reinterpret_cast<float*>(&val_32));
      case kLong:
        val_64 = Helper::strToId(str.substr(1, sizeof(double)));
        return TypedValue(*reinterpret_cast<long*>(&val_64));
      case kDouble:
        val_64 = Helper::strToId(str.substr(1, sizeof(double)));
        return TypedValue(*reinterpret_cast<double*>(&val_64));
      case kDatetime:
        return TypedValue(DatetimeLit{(std::int64_t)Helper::strToId(str.substr(1, sizeof(double)))});
      case kDatetimeInterval:
        return TypedValue(DatetimeIntervalLit{(std::int64_t)Helper::strToId(str.substr(1, sizeof(double)))});
      case kYearMonthInterval:
        return TypedValue(YearMonthIntervalLit{(std::int64_t)Helper::strToId(str.substr(1, sizeof(double)))});
      default:
        return TypedValue(0);
    }
    
    return TypedValue(0);
  }

  int Helper::valueLength(TypedValue value) {
    if (value.isNull()) {
      return 1;
    }
    TypeID type = value.getTypeID();
    if (type == kChar || type == kVarChar) {
      return value.getDataSize() + 9;
    }
    else {
      return value.getDataSize() + 1;
    }
  }

  bool Helper::valueEqual(TypedValue value1, TypedValue value2) {
    if (value1.isNull() || value2.isNull()) {
      return false;
    }
    if (value1.getTypeID() != value2.getTypeID()) {
      return false;
    }
    if (value1.getTypeID() == kChar || value2.getTypeID() == kVarChar) {
      return EqualComparison::Instance().compareTypedValuesChecked(value1, 
              TypeFactory::GetType(value1.getTypeID(), value1.getDataSize()), 
              value2, 
              TypeFactory::GetType(value2.getTypeID(), value2.getDataSize()));
    }
    else {
      return EqualComparison::Instance().compareTypedValuesChecked(value1, 
              TypeFactory::GetType(value1.getTypeID()), 
              value2, 
              TypeFactory::GetType(value2.getTypeID()));
    }
  }

} // namespace quickstep
