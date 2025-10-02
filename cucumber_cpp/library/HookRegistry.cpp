
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/TagExpression.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library
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

        auto Matches(const std::set<std::string, std::less<>>& tags)
        {
            return [&tags](const HookRegistryBase::Entry& entry)
            {
                return entry.tagExpression->Evaluate(tags);
            };
        }
    }

    HookBase::HookBase(Context& context)
        : context{ context }
    {}

    std::vector<HookMatch> HookRegistryBase::Query(HookType hookType, const std::set<std::string, std::less<>>& tags) const
    {
        std::vector<HookMatch> matches;

        for (const Entry& entry : registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)))
            matches.emplace_back(entry.factory);

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
