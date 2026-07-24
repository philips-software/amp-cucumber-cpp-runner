#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/StepType.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include "cucumber_cpp/library/util/StepFactory.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <ranges>
#include <set>
#include <source_location>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
    bool SourceLocationOrder::operator()(const std::source_location& lhs, const std::source_location& rhs) const
    {
        return std::make_tuple(std::string_view{ lhs.file_name() }, lhs.line()) < std::make_tuple(std::string_view{ rhs.file_name() }, rhs.line());
    }

    DefinitionRegistration& DefinitionRegistration::Instance()
    {
        static DefinitionRegistration instance;
        return instance;
    }

    void DefinitionRegistration::Clear()
    {
        registry.clear();
        customParameters.clear();
    }

    void DefinitionRegistration::TakeSnapshot()
    {
        if (!staticRegistry.empty() || !staticParameters.empty())
            return;

        staticRegistry.merge(registry);
        staticParameters.merge(customParameters);
    }

    void DefinitionRegistration::MergeInto(DefinitionRegistration& target)
    {
        if (this == &target)
            return;

        target.registry.merge(registry);
        target.customParameters.merge(customParameters);
    }

    void DefinitionRegistration::LoadIds(cucumber::gherkin::id_generator_ptr idGenerator)
    {
        const auto assignGenerator = [&idGenerator](auto& entry)
        {
            entry.id = idGenerator->next_id();
        };

        for (auto& [key, item] : staticRegistry)
            std::visit(assignGenerator, item);

        for (auto& [key, item] : registry)
            std::visit(assignGenerator, item);
    }

    std::vector<HookEntry> DefinitionRegistration::GetHooks()
    {
        std::vector<HookEntry> result;

        auto collectHooks = [&result](auto& reg)
        {
            for (auto& [key, entry] : reg)
                if (std::holds_alternative<HookEntry>(entry))
                    result.push_back(std::get<HookEntry>(entry));
        };

        collectHooks(staticRegistry);
        collectHooks(registry);
        return result;
    }

    std::set<cucumber_expression::CustomParameterEntry, std::less<>> DefinitionRegistration::GetRegisteredParameters() const
    {
        auto result = staticParameters;
        result.insert(customParameters.begin(), customParameters.end());
        return result;
    }

    cucumber_expression::ErasedConverter DefinitionRegistration::GetConverter(const std::string& name) const
    {
        auto findIn = [&name](const std::set<cucumber_expression::CustomParameterEntry, std::less<>>& params) -> cucumber_expression::ErasedConverter
        {
            auto it = std::ranges::find_if(params, [&name](const auto& entry)
                {
                    return entry.params.name == name;
                });
            if (it != params.end())
                return it->converter;

            return nullptr;
        };

        if (auto converter = findIn(staticParameters))
            return converter;

        return findIn(customParameters);
    }

    std::size_t DefinitionRegistration::Register(Hook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation)
    {
        registry.try_emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(GlobalHook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation)
    {
        registry.try_emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(std::string_view matcher, StepType stepType, util::StepFactory factory, std::source_location sourceLocation)
    {
        registry.try_emplace(sourceLocation, StepStringRegistration::Entry{ stepType, std::string{ matcher }, factory, sourceLocation });
        return registry.size();
    }
}
