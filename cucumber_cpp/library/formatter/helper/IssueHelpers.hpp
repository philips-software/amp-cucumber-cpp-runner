#ifndef HELPER_ISSUE_HELPERS_HPP
#define HELPER_ISSUE_HELPERS_HPP

#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <cstddef>
#include <ostream>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::ostream& FormatIssue(std::ostream& outputStream, std::size_t number, const TestCaseAttempt& testCaseAttempt, support::SupportCodeLibrary& supportCodeLibrary, bool printAttachments = true);
}

#endif
