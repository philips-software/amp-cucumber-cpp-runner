#ifndef SUPPORT_DEFINITION_REGISTRATION_HPP
#define SUPPORT_DEFINITION_REGISTRATION_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/StepType.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include "cucumber_cpp/library/util/StepFactory.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <ranges>
#include <set>
#include <source_location>
#include <string_view>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
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

        const std::set<cucumber_expression::CustomParameterEntry, std::less<>>& GetRegisteredParameters() const;

        template<class T>
        static std::size_t Register(Hook hook, util::HookType hookType, std::source_location sourceLocation = std::source_location::current());

        template<class T>
        static std::size_t Register(GlobalHook hook, util::HookType hookType, std::source_location sourceLocation = std::source_location::current());

        template<class T>
        static std::size_t Register(std::string_view matcher, StepType stepType, std::source_location sourceLocation = std::source_location::current());

        template<class Transformer, class TReturn>
        static std::size_t Register(cucumber_expression::CustomParameterEntryParams params, std::source_location location = std::source_location::current());

    private:
        std::size_t Register(Hook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation);
        std::size_t Register(GlobalHook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation);
        std::size_t Register(std::string_view matcher, StepType stepType, util::StepFactory factory, std::source_location sourceLocation);

        std::map<std::source_location, Entry, SourceLocationOrder> registry;
        std::set<cucumber_expression::CustomParameterEntry, std::less<>> customParameters;
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
    std::size_t DefinitionRegistration::Register(Hook hook, util::HookType hookType, std::source_location sourceLocation)
    {
        return Instance().Register(hook, hookType, util::HookBodyFactory<T>, sourceLocation);
    }

    template<class T>
    std::size_t DefinitionRegistration::Register(GlobalHook hook, util::HookType hookType, std::source_location sourceLocation)
    {
        return Instance().Register(hook, hookType, util::HookBodyFactory<T>, sourceLocation);
    }

    template<class T>
    std::size_t DefinitionRegistration::Register(std::string_view matcher, StepType stepType, std::source_location sourceLocation)
    {
        return Instance().Register(matcher, stepType, util::StepBodyFactory<T>, sourceLocation);
    }

    template<class Transformer, class TReturn>
    std::size_t DefinitionRegistration::Register(cucumber_expression::CustomParameterEntryParams params, std::source_location location)
    {
        auto& instance = Instance();
        instance.customParameters.emplace(params, instance.customParameters.size() + 1, location);

        cucumber_expression::ConverterTypeMap<TReturn>::Instance()[params.name] = Transformer::Transform;

        return instance.customParameters.size();
    }
}

#endif
