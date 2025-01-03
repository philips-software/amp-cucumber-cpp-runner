#include "cucumber_cpp/library/InternalError.hpp"
#include "cucumber_cpp/library/engine/StringTo.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <istream>
#include <string>

namespace cucumber_cpp::library::engine
{
    struct CustomType
    {
    };

    bool operator==(const CustomType& /*unused*/, const CustomType& /*unused*/)
    {
        return true;
    }

    std::istream& operator>>(std::istream& is, CustomType& /*unused*/)
    {
        return is;
    }

    TEST(TestStringTo, String)
    {
        const std::string s{ "Hello, World!" };
        const auto result = StringTo<std::string>(s);

        EXPECT_THAT(s, testing::StrEq(result));
    }

    TEST(TestStringTo, Boolean)
    {
        EXPECT_THAT(StringTo<bool>("true"), testing::Eq(true));
        EXPECT_THAT(StringTo<bool>("1"), testing::Eq(true));
        EXPECT_THAT(StringTo<bool>("yes"), testing::Eq(true));
        EXPECT_THAT(StringTo<bool>("on"), testing::Eq(true));
        EXPECT_THAT(StringTo<bool>("enabled"), testing::Eq(true));
        EXPECT_THAT(StringTo<bool>("active"), testing::Eq(true));

        EXPECT_THAT(StringTo<bool>("false"), testing::Eq(false));
        EXPECT_THAT(StringTo<bool>("0"), testing::Eq(false));
        EXPECT_THAT(StringTo<bool>("off"), testing::Eq(false));
        EXPECT_THAT(StringTo<bool>("disabled"), testing::Eq(false));
        EXPECT_THAT(StringTo<bool>("inactive"), testing::Eq(false));
        EXPECT_THAT(StringTo<bool>("parseerror"), testing::Eq(false));
    }

    TEST(TestStringTo, Int)
    {
        const std::string s{ "42" };
        const auto result = StringTo<int>(s);

        EXPECT_THAT(result, testing::Eq(42));
    }

    TEST(TestStringTo, Float)
    {
        const std::string s{ "3.14" };
        const auto result = StringTo<float>(s);

        EXPECT_THAT(result, testing::FloatEq(3.14F));
    }

    TEST(TestStringTo, CustomType)
    {
        const std::string s{ "Hello, World!" };
        const auto result = StringTo<CustomType>(s);

        EXPECT_THAT(result, testing::Eq(CustomType{}));
    }

    TEST(TestStringTo, ThrowsOnInvalidConversion)
    {
        EXPECT_THROW(StringTo<int>("Hello, World!"), InternalError);
    }
}
