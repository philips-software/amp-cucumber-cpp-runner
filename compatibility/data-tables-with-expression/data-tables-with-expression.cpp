#include "cucumber_cpp/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>

GIVEN(R"(a {string} with a table)", (const std::string& string))
{
    EXPECT_THAT(string, testing::StrEq("Cucumber"));

    ASSERT_THAT(dataTable, testing::IsTrue());
    ASSERT_THAT(dataTable->rows.size(), testing::Eq(1));
    ASSERT_THAT(dataTable->rows[0].cells.size(), testing::Eq(2));
    EXPECT_THAT(dataTable->rows[0].cells[0].value, testing::StrEq("Species"));
    EXPECT_THAT(dataTable->rows[0].cells[1].value, testing::StrEq("Cucumis sativus"));
}
