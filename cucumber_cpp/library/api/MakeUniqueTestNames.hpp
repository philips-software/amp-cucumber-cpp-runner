#ifndef API_MAKE_UNIQUE_TEST_NAMES_HPP
#define API_MAKE_UNIQUE_TEST_NAMES_HPP

#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include <functional>
#include <list>
#include <map>
#include <string>

namespace cucumber_cpp::library::api
{
    // Maps pickle ids to their hierarchical test name, duplicates get a ".#<n>" suffix in the order of pickleSources
    std::map<std::string, std::string, std::less<>> MakeUniqueTestNames(const cucumber::query::Query& query, const std::list<support::PickleSource>& pickleSources);
}

#endif
