#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/ContextManager.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::engine
{
    namespace TestRunner
    {
        struct PendingException
        {};

        struct StepNotFoundException
        {};

        struct AmbiguousStepException
        {};

        void Run(ContextManager& contextManager, std::vector<std::unique_ptr<FeatureInfo>> feature, report::ReportHandlerV2& reportHandler);
    }
}

#endif
