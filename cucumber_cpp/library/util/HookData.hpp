#ifndef UTIL_HOOK_DATA_HPP
#define UTIL_HOOK_DATA_HPP

#include <cstdint>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::util
{
    enum struct HookType : std::int8_t
    {
        beforeAll,
        afterAll,
        beforeFeature,
        afterFeature,
        before,
        after,
        beforeStep,
        afterStep,
    };

    struct HookData
    {
        std::string id;
        HookType type;
        std::optional<std::string_view> expression;
        std::optional<std::string_view> name;
        std::source_location sourceLocation;
    };
}

#endif
