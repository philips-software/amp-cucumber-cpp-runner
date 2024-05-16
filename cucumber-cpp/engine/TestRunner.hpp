#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber-cpp/engine/ContextManager.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include <memory>
#include <vector>

namespace cucumber_cpp::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::engine::TestRunner
{
    struct PendingException
    {};

    struct StepNotFoundException
    {};

    struct AmbiguousStepException
    {};

    void Run(ContextManager& contextManager, const std::vector<std::unique_ptr<FeatureInfo>>& feature, report::ReportHandlerV2& reportHandler);
}

#endif
