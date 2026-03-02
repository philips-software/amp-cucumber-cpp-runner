#include "cucumber_cpp/Steps.hpp"
#include <string>

// #ifdef INCLUDE_NLOHMANN_JSON_FWD_HPP_
// #error "Included nlohmann/json_fwd.hpp"
// #endif
// #ifdef INCLUDE_NLOHMANN_JSON_HPP_
// #error "Included nlohmann/json.hpp"
// #endif

STEP(R"(^a (.*?) with (.*?)$)", (const std::string& arg1, const std::string& arg2))
{
    // no-op
}

STEP(R"(^a step with (.*?)$)", (const std::string& arg1))
{
    // no-op
}
