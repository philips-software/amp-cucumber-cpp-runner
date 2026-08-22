#ifndef ASSEMBLE_ASSEMBLED_TEST_SUITE_HPP
#define ASSEMBLE_ASSEMBLED_TEST_SUITE_HPP

#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include <list>

namespace cucumber_cpp::library::assemble
{
    struct AssembledTestSuite
    {
        const cucumber::messages::GherkinDocument& gherkinDocument;
        std::list<AssembledTestCase> testCases;
    };
}

#endif
