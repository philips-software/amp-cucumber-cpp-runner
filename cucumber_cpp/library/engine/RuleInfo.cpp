#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::engine
{
    RuleInfo::RuleInfo(const struct FeatureInfo& featureInfo, std::string id, std::string title, std::string description, std::size_t line, std::size_t column)
        : featureInfo{ featureInfo }
        , id{ std::move(id) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , line{ line }
        , column{ column }
    {}

    const FeatureInfo& RuleInfo::FeatureInfo() const
    {
        return featureInfo;
    }

    [[nodiscard]] std::string_view RuleInfo::Id() const
    {
        return id;
    }

    const std::string& RuleInfo::Title() const
    {
        return title;
    }

    const std::string& RuleInfo::Description() const
    {
        return description;
    }

    const std::filesystem::path& RuleInfo::Path() const
    {
        return FeatureInfo().Path();
    }

    std::size_t RuleInfo::Line() const
    {
        return line;
    }

    std::size_t RuleInfo::Column() const
    {
        return column;
    }

    std::vector<std::unique_ptr<ScenarioInfo>>& RuleInfo::Scenarios()
    {
        return scenarios;
    }

    const std::vector<std::unique_ptr<ScenarioInfo>>& RuleInfo::Scenarios() const
    {
        return scenarios;
    }
}
