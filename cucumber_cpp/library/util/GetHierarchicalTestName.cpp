#include "cucumber_cpp/library/util/GetHierarchicalTestName.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/util/Trim.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <string>
#include <utility>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::string SanitizeName(std::string name)
        {
            std::ranges::replace(name, '\n', ' ');
            std::ranges::replace(name, '\r', ' ');
            std::ranges::replace(name, ';', ',');
            return Trim(name);
        }
    }

    std::string GetHierarchicalTestName(const cucumber::query::Query& query, const cucumber::messages::Pickle& pickle)
    {
        const auto lineageOpt = query.FindLineageBy(pickle);
        if (!lineageOpt.has_value() || lineageOpt->lineage == nullptr)
            return SanitizeName(pickle.name);

        const auto& lineage = *lineageOpt->lineage;
        const auto featureName = SanitizeName((lineage.feature != nullptr && !lineage.feature->name.empty()) ? lineage.feature->name : pickle.uri);
        const auto ruleName = SanitizeName(lineage.rule != nullptr ? lineage.rule->name : "");
        const auto scenarioName = SanitizeName(pickle.name);

        std::string exampleSuffix;
        if (lineage.example != nullptr)
        {
            if (lineage.examplesIndex.has_value() && lineage.exampleIndex.has_value())
                exampleSuffix = fmt::format(".#{}.{}", *lineage.examplesIndex + 1, *lineage.exampleIndex + 1);
            else if (lineage.exampleIndex.has_value())
                exampleSuffix = fmt::format(".#{}", *lineage.exampleIndex + 1);
        }

        if (!ruleName.empty())
            return fmt::format("{}.{}.{}{}", featureName, ruleName, scenarioName, exampleSuffix);

        return fmt::format("{}.{}{}", featureName, scenarioName, exampleSuffix);
    }
}
