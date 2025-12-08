#ifndef ASSEMBLE_ASSEMBLED_TEST_SUITE_HPP
#define ASSEMBLE_ASSEMBLED_TEST_SUITE_HPP

#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include <vector>

namespace cucumber_cpp::library::assemble
{
    struct AssembledTestSuite
    {
        const cucumber::messages::gherkin_document& gherkinDocument;
        std::vector<AssembledTestCase> testCases;
    };
}

#endif
