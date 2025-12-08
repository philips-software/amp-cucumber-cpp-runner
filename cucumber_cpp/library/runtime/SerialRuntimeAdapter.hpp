#ifndef RUNTIME_SERIAL_RUNTIME_ADAPTER_HPP
#define RUNTIME_SERIAL_RUNTIME_ADAPTER_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <span>
#include <string>

namespace cucumber_cpp::library::runtime
{
    struct SerialRuntimeAdapter : support::RuntimeAdapter
    {
        SerialRuntimeAdapter(std::string testRunStartedId,
            util::Broadcaster& broadcaster,
            cucumber::gherkin::id_generator_ptr idGenerator,
            const support::RunOptions::Runtime& options,
            support::SupportCodeLibrary supportCodeLibrary,
            Context& programContext);

        bool Run(std::span<const assemble::AssembledTestSuite> assembledTestSuites) override;

    private:
        std::string testRunStartedId;
        util::Broadcaster& broadcaster;
        cucumber::gherkin::id_generator_ptr idGenerator;
        const support::RunOptions::Runtime& options;
        support::SupportCodeLibrary supportCodeLibrary;
        Context& programContext;
    };
}

#endif
