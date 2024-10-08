#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <source_location>
#include <string>

namespace cucumber_cpp
{
    Step::Step(Context& context, const Table& table)
        : context{ context }
        , table{ table }
    {}

    void Step::Given(const std::string& step)
    {
        Any(StepType::given, step);
    }

    void Step::When(const std::string& step)
    {
        Any(StepType::when, step);
    }

    void Step::Then(const std::string& step)
    {
        Any(StepType::then, step);
    }

    void Step::Any(StepType type, const std::string& step)
    {
        const auto stepMatch = StepRegistry::Instance().Query(type, step);
        stepMatch.factory(context, {})->Execute(stepMatch.matches);
    }

    void Step::Pending(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepPending{ message, current };
    }
}
