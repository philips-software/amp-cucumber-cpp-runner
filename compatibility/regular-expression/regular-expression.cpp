#include "cucumber_cpp/CucumberCpp.hpp"
#include <optional>
#include <string>

STEP(R"(^a (.*?)(?: and a (.*?))?(?: and a (.*?))?$)", (const std::optional<std::string>& vegetable1, const std::optional<std::string>& vegetable2, const std::optional<std::string>& vegetable3))
{
    // no-op
}
