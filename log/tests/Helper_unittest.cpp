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

// Test the translation between TypedValue and string
TEST(HelperTest, TypedValueStringTest) {
  // Check int
  TypedValue pre_int_value(656478);
  TypedValue post_int_value = Helper::strToValue(Helper::valueToStr(pre_int_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_int_value, 
                                                                  TypeFactory::GetType(pre_int_value.getTypeID()), 
                                                                  post_int_value, 
                                                                  TypeFactory::GetType(post_int_value.getTypeID())));
  // Check float
  TypedValue pre_float_value((float) 1.023);
  TypedValue post_float_value = Helper::strToValue(Helper::valueToStr(pre_float_value));
  EXPECT_EQ(pre_float_value.getTypeID(), post_float_value.getTypeID());
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_float_value, 
                                                                  TypeFactory::GetType(pre_float_value.getTypeID()), 
                                                                  post_float_value, 
                                                                  TypeFactory::GetType(post_float_value.getTypeID())));
  // Check long
  TypedValue pre_long_value((long) 8984);
  TypedValue post_long_value = Helper::strToValue(Helper::valueToStr(pre_long_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_long_value, 
                                                                  TypeFactory::GetType(pre_long_value.getTypeID()), 
                                                                  post_long_value, 
                                                                  TypeFactory::GetType(post_long_value.getTypeID())));
  // Check double
  TypedValue pre_double_value((double) 564.58);
  TypedValue post_double_value = Helper::strToValue(Helper::valueToStr(pre_double_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_double_value, 
                                                                  TypeFactory::GetType(pre_double_value.getTypeID()), 
                                                                  post_double_value, 
                                                                  TypeFactory::GetType(post_double_value.getTypeID())));
  // Check Datetime
  TypedValue pre_datetime_value(DatetimeLit{(std::int64_t) 21872});
  TypedValue post_datetime_value = Helper::strToValue(Helper::valueToStr(pre_datetime_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_datetime_value, 
                                                                  TypeFactory::GetType(pre_datetime_value.getTypeID()), 
                                                                  post_datetime_value, 
                                                                  TypeFactory::GetType(post_datetime_value.getTypeID())));
  // Check DatetimeInterval
  TypedValue pre_datetime_interval_value(DatetimeIntervalLit{(std::int64_t) 9842});
  TypedValue post_datetime_interval_value = Helper::strToValue(Helper::valueToStr(pre_datetime_interval_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_datetime_interval_value, 
                                                                  TypeFactory::GetType(pre_datetime_interval_value.getTypeID()), 
                                                                  post_datetime_interval_value, 
                                                                  TypeFactory::GetType(post_datetime_interval_value.getTypeID())));
  // Check DatetimeInterval
  TypedValue pre_yearmonth_interval_value(YearMonthIntervalLit{(std::int64_t) -4028});
  TypedValue post_yearmonth_interval_value = Helper::strToValue(Helper::valueToStr(pre_yearmonth_interval_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_yearmonth_interval_value, 
                                                                  TypeFactory::GetType(pre_yearmonth_interval_value.getTypeID()), 
                                                                  post_yearmonth_interval_value, 
                                                                  TypeFactory::GetType(post_yearmonth_interval_value.getTypeID())));
  // Check char
  std::string char_str = "char type";
  TypedValue pre_char_value(kChar, char_str.c_str(), char_str.length());
  TypedValue post_char_value = Helper::strToValue(Helper::valueToStr(pre_char_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_char_value, 
                                                                  TypeFactory::GetType(pre_char_value.getTypeID(), pre_char_value.getDataSize()), 
                                                                  post_char_value, 
                                                                  TypeFactory::GetType(post_char_value.getTypeID(), post_char_value.getDataSize())));
  // Check varchar
  std::string varchar_str = "varchar type";
  TypedValue pre_varchar_value(kVarChar, varchar_str.c_str(), varchar_str.length() + 1);
  TypedValue post_varchar_value = Helper::strToValue(Helper::valueToStr(pre_varchar_value));
  EXPECT_EQ(true, 
            EqualComparison::Instance().compareTypedValuesChecked(pre_varchar_value, 
                                                                  TypeFactory::GetType(pre_varchar_value.getTypeID(), pre_varchar_value.getDataSize()), 
                                                                  post_varchar_value, 
                                                                  TypeFactory::GetType(post_varchar_value.getTypeID(), post_varchar_value.getDataSize())));
}


} // namespace quickstep
