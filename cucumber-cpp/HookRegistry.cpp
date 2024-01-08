
#include "cucumber-cpp/HookRegistry.hpp"
#include "cucumber-cpp/TagExpression.hpp"
#include <memory>

namespace cucumber_cpp
{
    namespace
    {
        auto TypeFilter(HookType hookType)
        {
            return [hookType](const HookRegistry::Entry& entry) -> bool
            {
                return entry.type == hookType;
            };
        };
    }

    HookBase::HookBase(Context& context)
        : context{ context }
    {}

    TagExpressionMatch::TagExpressionMatch(const std::string& tagExpression, const std::set<std::string>& tags)
        : matched{ IsTagExprSelected(tagExpression, tags) }
    {}

    bool TagExpressionMatch::Matched() const
    {
        return matched;
    }

    HookTagExpression::HookTagExpression(const std::string& tagExpression)
        : tagExpression{ tagExpression }
    {}

    std::unique_ptr<TagExpressionMatch> HookTagExpression::Match(const std::set<std::string>& tags) const
    {
        return std::make_unique<TagExpressionMatch>(tagExpression, tags);
    }

    std::string HookTagExpression::TagExpression() const
    {
        return tagExpression;
    }

    std::vector<HookMatch> HookRegistryBase::Query(HookType hookType, const std::set<std::string>& tags) const
    {
        std::vector<HookMatch> matches;

        for (const Entry& entry : registry | std::views::filter(TypeFilter(hookType)))
        {
            if (auto match = entry.hookTagExpression.Match(tags); match->Matched())
            {
                matches.emplace_back(std::move(match), entry.factory, entry.hookTagExpression);
            }
        }

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
