#ifndef RUNTIME_MAKE_RUNTIME_HPP
#define RUNTIME_MAKE_RUNTIME_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <memory>
#include <span>
#include <string>

namespace cucumber_cpp::library::runtime
{
    std::unique_ptr<support::RuntimeAdapter> MakeAdapter(const support::RunOptions::Runtime& options, std::string testRunStartedId, util::Broadcaster& broadcaster, support::SupportCodeLibrary supportCodeLibrary, cucumber::gherkin::id_generator_ptr idGenerator, Context& programContext);

    std::unique_ptr<support::Runtime> MakeRuntime(const support::RunOptions::Runtime& options, util::Broadcaster& broadcaster, std::span<const support::PickleSource> sourcedPickles, support::SupportCodeLibrary supportCodeLibrary, cucumber::gherkin::id_generator_ptr idGenerator, Context& programContext);
}

#endif
