#ifndef API_LOAD_PICKLES_HPP
#define API_LOAD_PICKLES_HPP

#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
#include <optional>

namespace cucumber_cpp::library::api
{
    // Collects pickles matching the tag expression, returns std::nullopt (after reporting) when any source fails to parse
    std::optional<std::list<support::PickleSource>> LoadPickles(const support::RunOptions::Sources& sources, const cucumber::gherkin::IdGeneratorPtr& idGenerator, util::Broadcaster& broadcaster);
}

#endif
