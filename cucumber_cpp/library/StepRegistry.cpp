#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>
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

    StepRegistry::StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry)
        : parameterRegistry{ parameterRegistry }
    {
        for (const auto& matcher : StepStringRegistration::Instance().GetEntries())
            Register(matcher.regex, matcher.type, matcher.factory);
    }

    StepMatch StepRegistry::Query(engine::StepType stepType, const std::string& expression)
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

    std::size_t StepRegistry::Size() const
    {
        return registry.size();
    }

    std::size_t StepRegistry::Size(engine::StepType stepType) const
    {
        return std::ranges::count(registry, stepType, &Entry::type);
    }

    std::vector<StepRegistry::EntryView> StepRegistry::List() const
    {
        std::vector<StepRegistry::EntryView> list;

        list.reserve(registry.size());

        for (const Entry& entry : registry)
            list.emplace_back(entry.regex, entry.used);

        return list;
    }

    void StepRegistry::Register(const std::string& matcher, engine::StepType stepType, std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table))
    {
        if (matcher.starts_with('^') || matcher.ends_with('$'))
            registry.emplace_back(stepType, cucumber_expression::Matcher{ std::in_place_type<cucumber_expression::RegularExpression>, matcher }, factory);
        else
            registry.emplace_back(stepType, cucumber_expression::Matcher{ std::in_place_type<cucumber_expression::Expression>, matcher, parameterRegistry }, factory);
    }

    StepStringRegistration& StepStringRegistration::Instance()
    {
        static StepStringRegistration instance;
        return instance;
    }

    std::span<StepStringRegistration::Entry> StepStringRegistration::GetEntries()
    {
        return registry;
    }

    std::span<const StepStringRegistration::Entry> StepStringRegistration::GetEntries() const
    {
        return registry;
    }
}
