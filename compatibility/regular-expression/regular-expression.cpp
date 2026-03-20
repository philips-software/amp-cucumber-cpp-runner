#include "cucumber_cpp/Steps.hpp"
#include <optional>
#include <string>

STEP(R"(^a (.*?)(?: and a (.*?))?(?: and a (.*?))?$)", ([[maybe_unused]] const std::optional<std::string>& vegetable1, [[maybe_unused]] const std::optional<std::string>& vegetable2, [[maybe_unused]] const std::optional<std::string>& vegetable3))
{
    // no-op
}
