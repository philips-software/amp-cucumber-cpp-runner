
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include <algorithm>
#include <cstddef>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library
{
    namespace
    {
        auto TypeFilter(HookType hookType)
        {
            return [hookType](const auto& keyValue)
            {
                return keyValue.second.type == hookType;
            };
        };

        auto Matches(std::span<const cucumber::messages::pickle_tag> tags)
        {
            return [tags](const auto& keyValue)
            {
                return keyValue.second.tagExpression->Evaluate(tags);
            };
        }

        auto Matches(std::span<const cucumber::messages::tag> tags)
        {
            return [tags](const auto& keyValue)
            {
                return keyValue.second.tagExpression->Evaluate(tags);
            };
        }
    }

    HookBase::HookBase(Context& context)
        : context{ context }
    {}

    HookRegistry::HookRegistry()
    {
        for (const auto& matcher : HookRegistration::Instance().GetEntries())
            Register(matcher.type, matcher.expression, matcher.factory, matcher.sourceLocation);
    }

    std::vector<std::string> HookRegistry::FindIds(HookType hookType, std::span<const cucumber::messages::pickle_tag> tags) const
    {
        auto ids = registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)) | std::views::keys;
        return { ids.begin(), ids.end() };
    }

    std::vector<std::string> HookRegistry::FindIds(HookType hookType, std::span<const cucumber::messages::tag> tags) const
    {
        auto ids = registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)) | std::views::keys;
        return { ids.begin(), ids.end() };
    }

    std::size_t HookRegistry::Size() const
    {
        return registry.size();
    }

    std::size_t HookRegistry::Size(HookType hookType) const
    {
        return std::ranges::count(registry | std::views::values, hookType, &Definition::type);
    }

    HookFactory HookRegistry::GetFactoryById(std::string id) const
    {
        return registry.at(id).factory;
    }

    const HookRegistry::Definition& HookRegistry::GetDefinitionById(std::string id) const
    {
        return registry.at(id);
    }

    void HookRegistry::Register(HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation)
    {
        auto id = std::to_string(nextId++);
        registry.emplace(id, Definition{ id, type, expression, factory, sourceLocation });
    }

    std::span<HookRegistration::Entry> HookRegistration::GetEntries()
    {
        return registry;
    }

    std::span<const HookRegistration::Entry> HookRegistration::GetEntries() const
    {
        return registry;
    }
}
