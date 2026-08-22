#ifndef API_GHERKIN_HPP
#define API_GHERKIN_HPP

#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>

namespace cucumber_cpp::library::api
{
    std::list<support::PickleSource> CollectPickles(const support::RunOptions::Sources& sources, cucumber::gherkin::IdGeneratorPtr idGenerator, const util::Broadcaster& broadcaster);
}

#endif
