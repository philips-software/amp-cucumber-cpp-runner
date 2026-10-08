#include "cucumber_cpp/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

GIVEN(R"(a step with a data table a doc string)")
{
    ASSERT_THAT(dataTable, testing::IsTrue());
    ASSERT_THAT(dataTable->rows.size(), testing::Eq(1));
    ASSERT_THAT(dataTable->rows[0].cells.size(), testing::Eq(1));
    EXPECT_THAT(dataTable->rows[0].cells[0].value, testing::StrEq("hello"));

    ASSERT_THAT(docString, testing::IsTrue());
    EXPECT_THAT(docString->content, testing::StrEq("world"));
}

GIVEN(R"(a step with a doc string a data table)")
{
    ASSERT_THAT(docString, testing::IsTrue());
    EXPECT_THAT(docString->content, testing::StrEq("hello"));

    ASSERT_THAT(dataTable, testing::IsTrue());
    ASSERT_THAT(dataTable->rows.size(), testing::Eq(1));
    ASSERT_THAT(dataTable->rows[0].cells.size(), testing::Eq(1));
    EXPECT_THAT(dataTable->rows[0].cells[0].value, testing::StrEq("world"));
}
