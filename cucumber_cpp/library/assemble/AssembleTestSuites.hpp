#ifndef ASSEMBLE_ASSEMBLE_TEST_SUITES_HPP
#define ASSEMBLE_ASSEMBLE_TEST_SUITES_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <span>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::assemble
{
    std::vector<AssembledTestSuite> AssembleTestSuites(
        support::SupportCodeLibrary supportCodeLibrary,
        std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        std::span<const support::PickleSource> sourcedPickles,
        cucumber::gherkin::id_generator_ptr idGenerator);
}

#endif
