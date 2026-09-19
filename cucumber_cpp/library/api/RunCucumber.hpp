#ifndef API_RUN_CUCUMBER_HPP
#define API_RUN_CUCUMBER_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cucumber/cucumber-expressions/ParameterRegistry.hpp>
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::library::api
{
    bool RunCucumber(const support::RunOptions& options, cucumber::cucumber_expressions::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string, std::less<>>& format, const std::string& formatOptions);
}

#endif
