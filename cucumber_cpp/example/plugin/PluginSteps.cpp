#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Steps.hpp"
#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include <string>

GIVEN(R"(the greeter says {string})", (std::string greeting))
{
    context.InsertAt("greeting", greeting);
}

WHEN(R"(the user responds)")
{
    const auto& greeting = context.Get<std::string>("greeting");
    context.InsertAt("response", std::string{ "You said: " + greeting });
}

THEN(R"(the conversation is complete)")
{
    [[maybe_unused]] const auto& response = context.Get<std::string>("response");
}

extern "C" CCR_EXPORT void ccr_register()
{
}
