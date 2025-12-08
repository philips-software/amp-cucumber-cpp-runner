#ifndef HELPER_SUMMARY_HELPERS_HPP
#define HELPER_SUMMARY_HELPERS_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include <span>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string FormatSummary(std::span<const TestCaseAttempt> testCaseAttempts, cucumber::messages::duration testRunDuration);
}

#endif
