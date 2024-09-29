#ifndef ENGINE_FEATUREINFO_HPP
#define ENGINE_FEATUREINFO_HPP

#include "cucumber_cpp/engine/RuleInfo.hpp"
#include "cucumber_cpp/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::engine
{
    struct FeatureInfo
    {
        FeatureInfo(std::set<std::string, std::less<>> tags, std::string title, std::string description, std::filesystem::path path, std::size_t line, std::size_t column);

        [[nodiscard]] const std::set<std::string, std::less<>>& Tags() const;
        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<std::unique_ptr<RuleInfo>>& Rules();
        [[nodiscard]] const std::vector<std::unique_ptr<RuleInfo>>& Rules() const;

        [[nodiscard]] std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios();
        [[nodiscard]] const std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios() const;

    private:
        std::set<std::string, std::less<>> tags;
        std::string title;
        std::string description;

        std::filesystem::path path;

        std::size_t line;
        std::size_t column;

        std::vector<std::unique_ptr<RuleInfo>> rules;
        std::vector<std::unique_ptr<ScenarioInfo>> scenarios;
    };
}

#endif
