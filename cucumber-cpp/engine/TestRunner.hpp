#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"

namespace cucumber_cpp::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::engine
{
    struct CurrentContext : Context
    {
        using Context::Context;

        [[nodiscard]] Result ExecutionStatus() const;

    protected:
        void ExecutionStatus(Result result);

    private:
        Result executionStatus{ Result::passed };
    };

    struct RunnerContext : CurrentContext
    {
        using CurrentContext::CurrentContext;
        using CurrentContext::ExecutionStatus;
    };

    namespace TestRunner
    {
        Result Run(RunnerContext& parent, const std::vector<FeatureInfo>& features, report::ReportHandlerV2& reportHandler);
    }
}

#endif
