#ifndef FORMATTER_TEST_CASE_ATTEMPT_FORMATTER_HPP
#define FORMATTER_TEST_CASE_ATTEMPT_FORMATTER_HPP

#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string FormatTestCaseAttempt(support::SupportCodeLibrary& supportCodeLibrary, const TestCaseAttempt& testCaseAttempt, bool printAttachments);
}

#endif
