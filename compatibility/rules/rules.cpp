#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include <cstdint>
#include <stack>
#include <string>

using Stock = std::stack<std::string>;

GIVEN(R"(the customer has {int} cents)", (std::int32_t money))
{
    context.InsertAt("money", money);
}

GIVEN(R"(there are chocolate bars in stock)")
{
    auto stock = context.Emplace<Stock>();
    stock->push("Mars");
}

GIVEN(R"(there are no chocolate bars in stock)")
{
    context.Emplace<Stock>();
}

WHEN(R"(the customer tries to buy a {int} cent chocolate bar)",(std::int32_t price))
{
    if (context.Get<std::int32_t>("money") >= price && !context.Get<Stock>().empty())
    {
        context.InsertAt("chocolate", context.Get<Stock>().top());
        context.Get<Stock>().pop();
    }
}

THEN(R"(the sale should not happen)")
{
    EXPECT_THAT(context.Contains("chocolate"), testing::IsFalse());
}

THEN(R"(the sale should happen)")
{
    EXPECT_THAT(context.Contains("chocolate"), testing::IsTrue());
}
