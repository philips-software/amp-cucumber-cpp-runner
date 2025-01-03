#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include <source_location>
#include <string>

namespace cucumber_cpp::library::engine
{
    Step::Step(Context& context, const Table& table)
        : context{ context }
        , table{ table }
    {}

    void Step::Given(const std::string& step)
    {
        // engine::TestRunner::Instance().ExecuteStep(StepType::given, step);
    }

    void Step::When(const std::string& step)
    {
        // engine::TestRunner::Instance().ExecuteStep(StepType::when, step);
    }

    void Step::Then(const std::string& step)
    {
        // engine::TestRunner::Instance().ExecuteStep(StepType::then, step);
    }

    void Step::Pending(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepPending{ message, current };
    }
}
