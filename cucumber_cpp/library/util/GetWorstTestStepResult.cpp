#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include <algorithm>
#include <span>
#include <type_traits>

namespace cucumber_cpp::library::util
{
    namespace
    {
        const auto to_underlying = [](const auto& value)
        {
            return static_cast<std::underlying_type_t<std::remove_cvref_t<decltype(value)>>>(value);
        };

        const auto compare = [](const cucumber::messages::TestStepResult& a, const cucumber::messages::TestStepResult& b)
        {
            return to_underlying(a.status) < to_underlying(b.status);
        };
    }

    cucumber::messages::TestStepResult GetWorstTestStepResult(std::span<const cucumber::messages::TestStepResult> testStepResults)
    {
        if (testStepResults.empty())
            return { .status = cucumber::messages::TestStepResultStatus::PASSED };

        return *std::ranges::max_element(testStepResults, compare);
    }
}
