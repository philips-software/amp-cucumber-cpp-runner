#ifndef ENGINE_FEATUREINFO_HPP
#define ENGINE_FEATUREINFO_HPP

#include "SourceInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber/messages/feature.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct FeatureInfo
    {
        FeatureInfo(cucumber::messages::feature, std::unique_ptr<struct SourceInfo>);

        [[nodiscard]] SourceInfo* SourceInfo() const;

        [[nodiscard]] std::set<std::string, std::less<>> Tags() const;
        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<std::unique_ptr<RuleInfo>>& Rules();
        [[nodiscard]] const std::vector<std::unique_ptr<RuleInfo>>& Rules() const;

        [[nodiscard]] std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios();
        [[nodiscard]] const std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios() const;

        [[nodiscard]] const cucumber::messages::feature& Pickle() const;

    private:

        cucumber::messages::feature feature;
        std::unique_ptr<struct SourceInfo> sourceInfo;

        std::vector<std::unique_ptr<RuleInfo>> rules;
        std::vector<std::unique_ptr<ScenarioInfo>> scenarios;
    };
}

#endif
