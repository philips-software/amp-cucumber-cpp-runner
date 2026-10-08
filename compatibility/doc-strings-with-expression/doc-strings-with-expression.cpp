#include "cucumber_cpp/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>

GIVEN(R"(a {string} with a doc string:)", (const std::string& string))
{
    EXPECT_THAT(string, testing::StrEq("Cucumber"));

    ASSERT_THAT(docString, testing::IsTrue());
    EXPECT_THAT(docString->content, testing::StrEq("Cucumis sativus"));
}
