
#include "cucumber_cpp/library/formatter/helper/IssueHelpers.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptFormatter.hpp"
#include "cucumber_cpp/library/support/Polyfill.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <cstddef>
#include <format>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    std::ostream& FormatIssue(std::ostream& outputStream, std::size_t number, const TestCaseAttempt& testCaseAttempt, support::SupportCodeLibrary& supportCodeLibrary, bool printAttachments)
    {
        using std::operator""sv;

        const auto prefix = std::format("{}) ", number);
        const auto formattedTestCaseAttempt = FormatTestCaseAttempt(supportCodeLibrary, testCaseAttempt, printAttachments);

        auto lines = formattedTestCaseAttempt | std::views::split("\n"sv);

        if (std::ranges::distance(lines) == 0)
            return outputStream;

        support::print(outputStream, "{}{}\n", prefix, std::string_view{ lines.front().begin(), lines.front().end() });

        for (const auto line : lines | std::views::drop(1))
            support::print(outputStream, "{:{}}{}\n", "", prefix.size(), std::string_view{ line.begin(), line.end() });

        return outputStream;
    }
}
