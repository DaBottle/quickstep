#include "log/Helper.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include "types/Type.hpp"
#include "types/TypeID.hpp"
#include "types/TypeFactory.hpp"
#include <limits>
#include <cstdint>

#include "gtest/gtest.h"

namespace quickstep {

// Test the translation between integer and string
TEST(HelperTest, IntStringTest) {
  EXPECT_EQ(0, (int) Helper::strToInt(Helper::intToStr(0)));
  EXPECT_EQ(5248, (int) Helper::strToInt(Helper::intToStr(5248)));
  EXPECT_EQ(-7128, (int) Helper::strToInt(Helper::intToStr(-7128)));
  EXPECT_EQ(std::numeric_limits<int>::min(), 
            (int) Helper::strToInt(Helper::intToStr(std::numeric_limits<int>::min())));
  EXPECT_EQ(std::numeric_limits<int>::max(), 
            (int) Helper::strToInt(Helper::intToStr(std::numeric_limits<int>::max())));
}

// Test the translation between id(uint64_t) and string
TEST(HelperTest, IdStringTest) {
  EXPECT_EQ((std::uint64_t) 0, Helper::strToId(Helper::idToStr(0)));
  EXPECT_EQ((std::uint64_t) 82938, Helper::strToId(Helper::idToStr(82938)));
  EXPECT_EQ(std::numeric_limits<std::uint64_t>::min(),
            Helper::strToId(Helper::idToStr(std::numeric_limits<std::uint64_t>::min())));
  EXPECT_EQ(std::numeric_limits<std::uint64_t>::max(),
            Helper::strToId(Helper::idToStr(std::numeric_limits<std::uint64_t>::max())));
}

// Test if the judgement of the equality between two TypedValue is correct
TEST(HelperTest, ValueEqualTest) {
  TypedValue int_value1((int) 1);
  TypedValue int_value2((int) 214);
  TypedValue long_value1((long) 5324);
  TypedValue long_value2((long) 123);
  std::string str1 = "varchar type";
  std::string str2 = "not equal";
  TypedValue str_value1(kVarChar, str1.c_str(), str1.length() + 1);
  TypedValue str_value2(kVarChar, str2.c_str(), str2.length() + 1);
  TypedValue null_value(kInt);
  
  // Equal
  EXPECT_TRUE(Helper::valueEqual(int_value1, int_value1));
  EXPECT_TRUE(Helper::valueEqual(long_value1, long_value1));
  EXPECT_TRUE(Helper::valueEqual(str_value1, str_value1));

  // Not Equal
  EXPECT_FALSE(Helper::valueEqual(int_value1, int_value2));
  EXPECT_FALSE(Helper::valueEqual(long_value1, long_value2));
  EXPECT_FALSE(Helper::valueEqual(str_value1, str_value2));

  // Different type and null
  EXPECT_FALSE(Helper::valueEqual(int_value1, long_value1));
  EXPECT_FALSE(Helper::valueEqual(int_value1, null_value));
  EXPECT_FALSE(Helper::valueEqual(null_value, null_value));
}

// Test the translation between TypedValue and string
TEST(HelperTest, TypedValueStringTest) {
  // Check int
  TypedValue pre_int_value(656478);
  TypedValue post_int_value = Helper::strToValue(Helper::valueToStr(pre_int_value));
  EXPECT_TRUE(Helper::valueEqual(pre_int_value, post_int_value));

  // Check float
  TypedValue pre_float_value((float) 1.023);
  TypedValue post_float_value = Helper::strToValue(Helper::valueToStr(pre_float_value));
  EXPECT_TRUE(Helper::valueEqual(pre_float_value, post_float_value));
  
  // Check long
  TypedValue pre_long_value((long) 8984);
  TypedValue post_long_value = Helper::strToValue(Helper::valueToStr(pre_long_value));
  EXPECT_TRUE(Helper::valueEqual(pre_long_value, post_long_value));
  
  // Check double
  TypedValue pre_double_value((double) 564.58);
  TypedValue post_double_value = Helper::strToValue(Helper::valueToStr(pre_double_value));
  EXPECT_TRUE(Helper::valueEqual(pre_double_value, post_double_value));
  
  // Check Datetime
  TypedValue pre_datetime_value(DatetimeLit{(std::int64_t) 21872});
  TypedValue post_datetime_value = Helper::strToValue(Helper::valueToStr(pre_datetime_value));
  EXPECT_TRUE(Helper::valueEqual(pre_datetime_value, post_datetime_value));
  
  // Check DatetimeInterval
  TypedValue pre_datetime_interval_value(DatetimeIntervalLit{(std::int64_t) 9842});
  TypedValue post_datetime_interval_value = Helper::strToValue(Helper::valueToStr(pre_datetime_interval_value));
  EXPECT_TRUE(Helper::valueEqual(pre_datetime_interval_value, post_datetime_interval_value));
  
  // Check DatetimeInterval
  TypedValue pre_yearmonth_interval_value(YearMonthIntervalLit{(std::int64_t) -4028});
  TypedValue post_yearmonth_interval_value = Helper::strToValue(Helper::valueToStr(pre_yearmonth_interval_value));
  EXPECT_TRUE(Helper::valueEqual(pre_yearmonth_interval_value, post_yearmonth_interval_value));
  
  // Check char
  std::string char_str = "char type";
  TypedValue pre_char_value(kChar, char_str.c_str(), char_str.length());
  TypedValue post_char_value = Helper::strToValue(Helper::valueToStr(pre_char_value));
  EXPECT_TRUE(Helper::valueEqual(pre_char_value, post_char_value));
  
  // Check varchar
  std::string varchar_str = "varchar type";
  TypedValue pre_varchar_value(kVarChar, varchar_str.c_str(), varchar_str.length() + 1);
  TypedValue post_varchar_value = Helper::strToValue(Helper::valueToStr(pre_varchar_value));
  EXPECT_TRUE(Helper::valueEqual(pre_varchar_value, post_varchar_value));
}

// Test if the length of TypedValue in log is calculated correctly
TEST(HelperTest, ValueLengthTest) {
  // Check 32-bit, 64-bit, string and null
  TypedValue int_value((int) 28394);
  TypedValue long_value((long) -9295);
  std::string char_str = "char value";
  TypedValue char_value(kChar, char_str.c_str(), char_str.length());
  TypedValue null_value(kFloat);
  
  EXPECT_EQ((int) sizeof(int) + 1, Helper::valueLength(int_value));
  EXPECT_EQ((int) sizeof(long) + 1, Helper::valueLength(long_value));
  EXPECT_EQ((int) char_str.length() + 9, Helper::valueLength(char_value));
  EXPECT_EQ(1, Helper::valueLength(null_value));
}



} // namespace quickstep
