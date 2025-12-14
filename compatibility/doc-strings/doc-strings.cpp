#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include <string>

STEP(R"(a doc string:)")
{
    ASSERT_THAT(docString, testing::IsTrue());
    ASSERT_THAT(docString->content, testing::Not(testing::IsEmpty()));
}
