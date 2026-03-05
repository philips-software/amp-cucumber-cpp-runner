#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/StepType.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include "cucumber_cpp/library/util/StepFactory.hpp"
#include <cstddef>
#include <filesystem>
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

#if defined(CCR_STANDALONE)
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include "fmt/std.h"
#endif

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

    void DefinitionRegistration::LoadIds(cucumber::gherkin::id_generator_ptr idGenerator)
    {
        const auto assignGenerator = [&idGenerator](auto& entry)
        {
            entry.id = idGenerator->next_id();
        };

        for (auto& [key, item] : registry)
            std::visit(assignGenerator, item);
    }

    std::vector<HookEntry> DefinitionRegistration::GetHooks()
    {
        auto allSteps = registry | std::views::values | std::views::filter([](const Entry& entry)
                                                            {
                                                                return std::holds_alternative<HookEntry>(entry);
                                                            }) |
                        std::views::transform([](const Entry& entry)
                            {
                                return std::get<HookEntry>(entry);
                            });
        return { allSteps.begin(), allSteps.end() };
    }

    const std::set<cucumber_expression::CustomParameterEntry, std::less<>>& DefinitionRegistration::GetRegisteredParameters() const
    {
        return customParameters;
    }

    namespace
    {
        void PrintContents(std::string_view type, std::source_location sourceLocation, const std::map<std::source_location, Entry, SourceLocationOrder>& registry)
        {
#if defined(CCR_STANDALONE)
            fmt::println("Added ({}): {}:{}", type, std::filesystem::path{ sourceLocation.file_name() }, sourceLocation.line());
            fmt::println("Registry contents:");
            for (const auto& [key, item] : registry)
                fmt::println("  {}:{}", std::filesystem::path{ key.file_name() }, key.line());
#endif
        }
    }

    std::size_t DefinitionRegistration::Register(Hook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        PrintContents("Hook", sourceLocation, registry);
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(GlobalHook hook, util::HookType hookType, util::HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        PrintContents("GlobalHook", sourceLocation, registry);
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(std::string_view matcher, StepType stepType, util::StepFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, StepStringRegistration::Entry{ stepType, std::string{ matcher }, factory, sourceLocation });
        PrintContents("Step", sourceLocation, registry);
        return registry.size();
    }
}
