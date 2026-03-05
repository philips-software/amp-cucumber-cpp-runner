#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
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

        const auto compare = [](const cucumber::messages::test_step_result& a, const cucumber::messages::test_step_result& b)
        {
            return to_underlying(a.status) < to_underlying(b.status);
        };
    }

    cucumber::messages::test_step_result GetWorstTestStepResult(std::span<const cucumber::messages::test_step_result> testStepResults)
    {
        if (testStepResults.empty())
            return { .status = cucumber::messages::test_step_result_status::PASSED };

        return *std::ranges::max_element(testStepResults, compare);
    }
}
