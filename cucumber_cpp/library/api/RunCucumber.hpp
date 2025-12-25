#ifndef API_RUN_CUCUMBER_HPP
#define API_RUN_CUCUMBER_HPP

#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <set>
#include <string>

namespace cucumber_cpp::library::api
{
    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string>& format, const std::string& formatOptions);
}

#endif
