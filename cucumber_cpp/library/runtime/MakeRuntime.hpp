#ifndef RUNTIME_MAKE_RUNTIME_HPP
#define RUNTIME_MAKE_RUNTIME_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
#include <memory>

namespace cucumber_cpp::library::runtime
{
    std::unique_ptr<support::Runtime> MakeRuntime(const support::RunOptions::Runtime& options, util::Broadcaster& broadcaster, const std::list<support::PickleSource>& sourcedPickles, support::SupportCodeLibrary& supportCodeLibrary, cucumber::gherkin::id_generator_ptr idGenerator, Context& programContext);
}

#endif
