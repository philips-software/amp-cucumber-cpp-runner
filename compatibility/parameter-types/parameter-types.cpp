#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include <string>

struct Flight
{
    std::string from;
    std::string to;
};

PARAMETER(Flight, "flight", "([A-Z]{3})-([A-Z]{3})", true)
{
    return { group.children[0].value.value(), group.children[1].value.value() };
}

STEP(R"({flight} has been delayed)", (const Flight& flight))
{
    EXPECT_THAT(flight.from, testing::StrEq("LHR"));
    EXPECT_THAT(flight.to, testing::StrEq("CDG"));
}
