#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include <source_location>
#include <string>

namespace cucumber_cpp::library::engine
{
    Step::Step(Context& context, const Table& table, const std::string& docString)
        : context{ context }
        , table{ table }
        , docString{ docString }
    {}

    void Step::Given(const std::string& step) const
    {
        TestRunner::Instance().NestedStep(StepType::given, step);
    }

    void Step::When(const std::string& step) const
    {
        TestRunner::Instance().NestedStep(StepType::when, step);
    }

    void Step::Then(const std::string& step) const
    {
        TestRunner::Instance().NestedStep(StepType::then, step);
    }

    void Step::Pending(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepPending{ message, current };
    }
}
