#include "cucumber_cpp/library/formatter/helper/PickleParser.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include <algorithm>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        constexpr auto toPair = [](const cucumber::messages::pickle_step& step) -> std::pair<std::string, const cucumber::messages::pickle_step&>
        {
            return { step.id, step };
        };
    }

    std::map<std::string, const cucumber::messages::pickle_step&> GetPickleStepMap(const cucumber::messages::pickle& pickle)
    {
        auto range = pickle.steps | std::views::transform(toPair);
        return { range.begin(), range.end() };
    }

    std::string_view GetStepKeyword(const cucumber::messages::pickle_step& pickleStep, const GherkinStepMap& gherkinStepMap)
    {
        auto first = std::ranges::find_if(pickleStep.ast_node_ids, [&gherkinStepMap](const std::string& id)
            {
                return gherkinStepMap.contains(id);
            });
        return gherkinStepMap.at(*first).keyword;
    }
}
