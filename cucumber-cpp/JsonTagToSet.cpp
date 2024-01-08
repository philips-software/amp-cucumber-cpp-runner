#include "cucumber-cpp/JsonTagToSet.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ranges>
#include <set>
#include <string>

namespace cucumber_cpp
{
    std::set<std::string> JsonTagsToSet(const nlohmann::json& json)
    {
        static const auto getName = [](const nlohmann::json& json)
        {
            return json["name"];
        };

        std::set<std::string> tagSet;
        std::ranges::copy(json | std::views::transform(getName), std::inserter(tagSet, tagSet.end()));

        return tagSet;
    }
}
