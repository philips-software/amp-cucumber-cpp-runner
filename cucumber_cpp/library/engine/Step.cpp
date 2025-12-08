#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber/messages/pickle_step_type.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
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
        // CucumberTestServer::Instance()->RunStep(step, cucumber::messages::pickle_step_type::CONTEXT);
    }

    void Step::When(const std::string& step) const
    {
        // CucumberTestServer::Instance()->RunStep(step, cucumber::messages::pickle_step_type::ACTION);
    }

    void Step::Then(const std::string& step) const
    {
        // CucumberTestServer::Instance()->RunStep(step, cucumber::messages::pickle_step_type::OUTCOME);
    }

    void Step::Pending(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepPending{ message, current };
    }
}
