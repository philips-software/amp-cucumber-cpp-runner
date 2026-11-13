#include "cucumber_cpp/Steps.hpp"
#include <string>

GIVEN(R"(the greeter says {string})", (std::string greeting))
{
    context.InsertAt("greeting", greeting);
}

WHEN(R"(the user responds)")
{
    const auto& greeting = context.GetAt<std::string>("greeting");
    context.InsertAt("response", std::string{ "You said: " + greeting });
}

THEN(R"(the conversation is complete)")
{
    [[maybe_unused]] const auto& response = context.GetAt<std::string>("response");
}
