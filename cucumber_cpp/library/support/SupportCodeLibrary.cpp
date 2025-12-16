#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include <compare>
#include <cstddef>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace std
{
    std::strong_ordering operator<=>(const std::source_location& left, const std::source_location& right)
    {
        return std::forward_as_tuple(left.file_name(), left.line()) <=> std::forward_as_tuple(right.file_name(), right.line());
    }
}

namespace cucumber_cpp::library
{
    std::strong_ordering operator<=>(const HookRegistration::Entry& left, const HookRegistration::Entry& right)
    {
        return left.sourceLocation <=> right.sourceLocation;
    }

    std::strong_ordering operator<=>(const StepStringRegistration::Entry& left, const StepStringRegistration::Entry& right)
    {
        return left.sourceLocation <=> right.sourceLocation;
    }
}

namespace cucumber_cpp::library::support
{
    std::strong_ordering operator<=>(const Entry& left, const Entry& right)
    {
        constexpr static auto sourceLocationVisitor = [](const auto& entry)
        {
            return entry.sourceLocation;
        };

        const auto& leftSource = std::visit(sourceLocationVisitor, left);
        const auto& rightSource = std::visit(sourceLocationVisitor, right);

        return leftSource <=> rightSource;
    }

    DefinitionRegistration& DefinitionRegistration::Instance()
    {
        static DefinitionRegistration instance;
        return instance;
    }

    void DefinitionRegistration::LoadIds(cucumber::gherkin::id_generator_ptr idGenerator)
    {
        static auto assignGenerator = [&idGenerator](auto& entry)
        {
            entry.id = idGenerator->next_id();
        };

        for (auto& [key, item] : registry)
            std::visit(assignGenerator, item);
    }

    std::vector<StepStringRegistration::Entry> DefinitionRegistration::GetSteps()
    {
        auto allSteps = registry | std::views::values | std::views::filter([](const Entry& entry)
                                                            {
                                                                return std::holds_alternative<StepStringRegistration::Entry>(entry);
                                                            }) |
                        std::views::transform([](const Entry& entry)
                            {
                                return std::get<StepStringRegistration::Entry>(entry);
                            });
        return { allSteps.begin(), allSteps.end() };
    }

    std::vector<HookRegistration::Entry> DefinitionRegistration::GetHooks()
    {
        auto allSteps = registry | std::views::values | std::views::filter([](const Entry& entry)
                                                            {
                                                                return std::holds_alternative<HookRegistration::Entry>(entry);
                                                            }) |
                        std::views::transform([](const Entry& entry)
                            {
                                return std::get<HookRegistration::Entry>(entry);
                            });
        return { allSteps.begin(), allSteps.end() };
    }

    std::size_t DefinitionRegistration::Register(Hook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookRegistration::Entry{ hookType, hook.tagExpression, factory, sourceLocation });
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(GlobalHook hook, HookType hookType, HookFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, HookRegistration::Entry{ hookType, "", factory, sourceLocation });
        return registry.size();
    }

    std::size_t DefinitionRegistration::Register(std::string_view matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation)
    {
        registry.emplace(sourceLocation, StepStringRegistration::Entry{ stepType, std::string{ matcher }, factory, sourceLocation });
        return registry.size();
    }
}
