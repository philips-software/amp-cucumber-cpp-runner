#ifndef ASSEMBLE_ASSEMBLED_TEST_CASE_HPP
#define ASSEMBLE_ASSEMBLED_TEST_CASE_HPP

#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/TestCase.hpp"

namespace cucumber_cpp::library::assemble
{
    struct AssembledTestCase
    {
        const cucumber::messages::Pickle& pickle;
        const cucumber::messages::TestCase testCase;
    };
}

#endif
