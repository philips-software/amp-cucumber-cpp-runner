
#include "cucumber_cpp/HookRegistry.hpp"
#include "cucumber_cpp/Context.hpp"
#include "cucumber_cpp/TagExpression.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    namespace
    {
        auto TypeFilter(HookType hookType)
        {
            return [hookType](const HookRegistry::Entry& entry)
            {
                return entry.type == hookType;
            };
        };
    }

    HookBase::HookBase(Context& context)
        : context{ context }
    {}

    TagExpressionMatch::TagExpressionMatch(const std::string& tagExpression, const std::set<std::string, std::less<>>& tags)
        : matched{ IsTagExprSelected(tagExpression, tags) }
    {}

    bool TagExpressionMatch::Matched() const
    {
        return matched;
    }

    HookTagExpression::HookTagExpression(std::string tagExpression)
        : tagExpression{ std::move(tagExpression) }
    {}

    std::unique_ptr<TagExpressionMatch> HookTagExpression::Match(const std::set<std::string, std::less<>>& tags) const
    {
        return std::make_unique<TagExpressionMatch>(tagExpression, tags);
    }

    std::string HookTagExpression::TagExpression() const
    {
        return tagExpression;
    }

    std::vector<HookMatch> HookRegistryBase::Query(HookType hookType, const std::set<std::string, std::less<>>& tags) const
    {
        std::vector<HookMatch> matches;

        for (const Entry& entry : registry | std::views::filter(TypeFilter(hookType)))
            if (auto match = entry.hookTagExpression.Match(tags); match->Matched())
                matches.emplace_back(std::move(match), entry.factory, entry.hookTagExpression);

        return matches;
    }

    std::size_t HookRegistryBase::Size() const
    {
        return registry.size();
    }

    std::size_t HookRegistryBase::Size(HookType hookType) const
    {
        return std::ranges::count(registry, hookType, &Entry::type);
    }

    HookRegistry& HookRegistry::Instance()
    {
        static HookRegistry instance;
        return instance;
    }
}
