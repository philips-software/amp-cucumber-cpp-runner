#ifndef SUPPORT_SUPPORT_CODE_LIBRARY_HPP
#define SUPPORT_SUPPORT_CODE_LIBRARY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
    struct GlobalHook

    {
        std::string_view name{};
        std::int32_t order{ 0 };
    };

    struct Hook
    {
        std::string_view expression{ "" };
        std::string_view name{};
        std::int32_t order{ 0 };
    };

    struct HookEntry
    {
        HookEntry(HookType type, GlobalHook hook, HookFactory factory, std::source_location sourceLocation)
            : type{ type }
            , expression{ std::nullopt }
            , name{ hook.name.empty() ? std::nullopt : std::make_optional(hook.name) }
            , order{ hook.order }
            , factory{ factory }
            , sourceLocation{ sourceLocation }
        {}

        HookEntry(HookType type, Hook hook, HookFactory factory, std::source_location sourceLocation)
            : type{ type }
            , expression{ hook.expression.empty() ? std::nullopt : std::make_optional(hook.expression) }
            , name{ hook.name.empty() ? std::nullopt : std::make_optional(hook.name) }
            , order{ hook.order }
            , factory{ factory }
            , sourceLocation{ sourceLocation }
        {}

        HookType type;
        std::optional<std::string_view> expression;
        std::optional<std::string_view> name;
        std::int32_t order;
        HookFactory factory;
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

    struct DefinitionRegistration
    {
    private:
        DefinitionRegistration() = default;

    public:
        static DefinitionRegistration& Instance();

        void LoadIds(cucumber::gherkin::id_generator_ptr idGenerator);

        template<class T>
        void ForEachRegisteredStep(const T& func);

        std::vector<HookEntry> GetHooks();

        template<class T>
        static std::size_t Register(Hook hook, HookType hookType, std::source_location sourceLocation = std::source_location::current());

        template<class T>
        static std::size_t Register(GlobalHook hook, HookType hookType, std::source_location sourceLocation = std::source_location::current());

        template<class T>
        static std::size_t Register(std::string_view matcher, engine::StepType stepType, std::source_location sourceLocation = std::source_location::current());

    private:
        std::size_t Register(Hook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation);
        std::size_t Register(GlobalHook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation);
        std::size_t Register(std::string_view matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation);

        std::map<std::source_location, Entry, SourceLocationOrder> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    void DefinitionRegistration::ForEachRegisteredStep(const T& func)
    {
        auto allSteps = registry |
                        std::views::values |
                        std::views::filter([](const Entry& entry)
                            {
                                return std::holds_alternative<StepStringRegistration::Entry>(entry);
                            }) |
                        std::views::transform([](const Entry& entry)
                            {
                                return std::get<StepStringRegistration::Entry>(entry);
                            });

        for (const auto& step : allSteps)
            func(step);
    }

    template<class T>
    std::size_t DefinitionRegistration::Register(Hook hook, HookType hookType, std::source_location sourceLocation)
    {
        return Instance().Register(hook, hookType, HookBodyFactory<T>, sourceLocation);
    }

    template<class T>
    std::size_t DefinitionRegistration::Register(GlobalHook hook, HookType hookType, std::source_location sourceLocation)
    {
        return Instance().Register(hook, hookType, HookBodyFactory<T>, sourceLocation);
    }

    template<class T>
    std::size_t DefinitionRegistration::Register(std::string_view matcher, engine::StepType stepType, std::source_location sourceLocation)
    {
        return Instance().Register(matcher, stepType, StepBodyFactory<T>, sourceLocation);
    }
}

#endif
