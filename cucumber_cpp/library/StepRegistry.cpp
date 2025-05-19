#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    namespace
    {
        auto TypeFilter(engine::StepType stepType)
        {
            return [stepType](const StepRegistry::Entry& entry)
            {
                return entry.type == stepType || entry.type == engine::StepType::any;
            };
        };
    }

    StepRegistry& StepRegistry::Instance()
    {
        static StepRegistry instance;
        return instance;
    }

    StepMatch StepRegistryBase::Query(engine::StepType stepType, const std::string& expression)
    {
        std::vector<StepMatch> matches;

        for (Entry& entry : registry | std::views::filter(TypeFilter(stepType)))
        {
            auto match = std::visit(cucumber_expression::MatchVisitor{ expression }, entry.regex);
            if (match)
            {
                matches.emplace_back(entry.factory, *match, std::visit(cucumber_expression::PatternVisitor{}, entry.regex));
                ++entry.used;
            }
        }

        if (matches.empty())
            throw StepNotFoundError{};

        if (matches.size() > 1)
            throw AmbiguousStepError{ std::move(matches) };

        return std::move(matches.front());
    }

    std::size_t StepRegistryBase::Size() const
    {
        return registry.size();
    }

    std::size_t StepRegistryBase::Size(engine::StepType stepType) const
    {
        return std::ranges::count(registry, stepType, &Entry::type);
    }

    std::vector<StepRegistryBase::EntryView> StepRegistryBase::List() const
    {
        std::vector<StepRegistryBase::EntryView> list;

        list.reserve(registry.size());

        for (const Entry& entry : registry)
            list.emplace_back(entry.regex, entry.used);

        return list;
    }
}
