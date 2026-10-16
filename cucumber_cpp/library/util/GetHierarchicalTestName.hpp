#ifndef UTIL_GET_HIERARCHICAL_TEST_NAME_HPP
#define UTIL_GET_HIERARCHICAL_TEST_NAME_HPP

#include "cucumber/messages/Pickle.hpp"
#include "cucumber/query/Query.hpp"
#include <string>

namespace cucumber_cpp::library::util
{
    std::string GetHierarchicalTestName(const cucumber::query::Query& query, const cucumber::messages::Pickle& pickle);
}

#endif
