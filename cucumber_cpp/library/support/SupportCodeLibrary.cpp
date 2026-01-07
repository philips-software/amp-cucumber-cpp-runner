#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include <cstddef>
#include <map>
#include <ranges>
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
        return std::forward_as_tuple(lhs.file_name(), lhs.line()) < std::forward_as_tuple(rhs.file_name(), rhs.line());
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

    namespace
    {
        void PrintContents(std::string_view type, std::source_location sourceLocation, const std::map<std::source_location, Entry, SourceLocationOrder>& registry)
        {
            std::cout << std::format("Added ({}): {}:{}\n", type, sourceLocation.file_name(), sourceLocation.line());
            std::cout << "Registry contents:\n";
            for (const auto& [key, item] : registry)
                std::cout << std::format("  {}:{}\n", key.file_name(), key.line());
        }
    }

    std::size_t DefinitionRegistration::Register(Hook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        PrintContents("Hook", sourceLocation, registry);
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(GlobalHook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookEntry{ hookType, hook, factory, sourceLocation });
        PrintContents("GlobalHook", sourceLocation, registry);
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(std::string_view matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, StepStringRegistration::Entry{ stepType, std::string{ matcher }, factory, sourceLocation });
        PrintContents("Step", sourceLocation, registry);
        return registry.size();
    }
}
