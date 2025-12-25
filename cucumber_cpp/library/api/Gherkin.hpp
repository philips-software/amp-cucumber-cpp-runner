#ifndef API_GHERKIN_HPP
#define API_GHERKIN_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>

namespace cucumber_cpp::library::api
{
    std::list<support::PickleSource> CollectPickles(const support::RunOptions::Sources& sources, cucumber::gherkin::id_generator_ptr idGenerator, util::Broadcaster& broadcaster);
}

#endif
