#ifndef ENGINE_SCENARIOINFO_HPP
#define ENGINE_SCENARIOINFO_HPP

#include "cucumber-cpp/engine/AmbiguousStepInfo.hpp"
#include "cucumber-cpp/engine/MissingStepInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace cucumber_cpp::engine
{
    struct FeatureInfo;

    struct ScenarioInfo
    {
        ScenarioInfo(const FeatureInfo& featureInfo, std::vector<std::string> tags, std::string title, std::string description, std::size_t line, std::size_t column);

        [[nodiscard]] const struct FeatureInfo& FeatureInfo() const;

        [[nodiscard]] const std::vector<std::string>& Tags() const;
        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<StepInfo>& Children();
        [[nodiscard]] const std::vector<StepInfo>& Children() const;

        [[nodiscard]] std::vector<MissingStepInfo>& MissingChildren();
        [[nodiscard]] const std::vector<MissingStepInfo>& MissingChildren() const;

        [[nodiscard]] std::vector<AmbiguousStepInfo>& AmbiguousChildren();
        [[nodiscard]] const std::vector<AmbiguousStepInfo>& AmbiguousChildren() const;

    private:
        const struct FeatureInfo& featureInfo;

        std::vector<std::string> tags;
        std::string title;
        std::string description;

        std::size_t line;
        std::size_t column;

        std::vector<StepInfo> children;
        std::vector<MissingStepInfo> missingChildren;
        std::vector<AmbiguousStepInfo> ambiguousChildren;
    };
}

#endif
