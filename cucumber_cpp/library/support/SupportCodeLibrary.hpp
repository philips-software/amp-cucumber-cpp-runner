#ifndef SUPPORT_SUPPORT_CODE_LIBRARY_HPP
#define SUPPORT_SUPPORT_CODE_LIBRARY_HPP

#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include <cstdint>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <variant>

namespace cucumber_cpp::library::support
{
    struct UndefinedParameters;

    struct GlobalHook
    {
        std::string_view name;
        std::int32_t order{ 0 };
    };

    struct Hook
    {
        std::string_view expression;
        std::string_view name;
        std::int32_t order{ 0 };
    };

    struct HookEntry
    {
        HookEntry(util::HookType type, GlobalHook hook, util::HookFactory factory, std::source_location sourceLocation)
            : type{ type }
            , expression{ std::nullopt }
            , name{ hook.name.empty() ? std::nullopt : std::make_optional(hook.name) }
            , order{ hook.order }
            , factory{ factory }
            , sourceLocation{ sourceLocation }
        {}

        HookEntry(util::HookType type, Hook hook, util::HookFactory factory, std::source_location sourceLocation)
            : type{ type }
            , expression{ hook.expression.empty() ? std::nullopt : std::make_optional(hook.expression) }
            , name{ hook.name.empty() ? std::nullopt : std::make_optional(hook.name) }
            , order{ hook.order }
            , factory{ factory }
            , sourceLocation{ sourceLocation }
        {}

        util::HookType type;
        std::optional<std::string_view> expression;
        std::optional<std::string_view> name;
        std::int32_t order;
        util::HookFactory factory;
        std::source_location sourceLocation;
        std::string id{ "unassigned" };
    };

    struct SupportCodeLibrary
    {
        HookRegistry& hookRegistry;
        StepRegistry& stepRegistry;
        cucumber_expression::ParameterRegistry& parameterRegistry;
        UndefinedParameters& undefinedParameters;
    };

    using Entry = std::variant<HookEntry, StepStringRegistration::Entry>;

    struct SourceLocationOrder
    {
        bool operator()(const std::source_location& lhs, const std::source_location& rhs) const;
    };
}

#endif
