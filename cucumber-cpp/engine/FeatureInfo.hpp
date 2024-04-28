#ifndef ENGINE_FEATUREINFO_HPP
#define ENGINE_FEATUREINFO_HPP

#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <vector>

namespace cucumber_cpp::engine
{
    struct FeatureInfo
    {
        FeatureInfo(std::vector<std::string> tags, std::string title, std::string description, std::filesystem::path path, std::size_t line, std::size_t column);

        [[nodiscard]] const std::vector<std::string>& Tags() const;
        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<ScenarioInfo>& Children();
        [[nodiscard]] const std::vector<ScenarioInfo>& Children() const;

    private:
        std::vector<std::string> tags;
        std::string title;
        std::string description;

        std::filesystem::path path;

        std::size_t line;
        std::size_t column;

        std::vector<ScenarioInfo> children;
    };
}

#endif
