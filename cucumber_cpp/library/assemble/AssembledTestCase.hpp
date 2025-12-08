#ifndef ASSEMBLE_ASSEMBLED_TEST_CASE_HPP
#define ASSEMBLE_ASSEMBLED_TEST_CASE_HPP

#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/test_case.hpp"

namespace cucumber_cpp::library::assemble
{
    struct AssembledTestCase
    {
        const cucumber::messages::pickle& pickle;
        const cucumber::messages::test_case testCase;
    };
}

#endif
