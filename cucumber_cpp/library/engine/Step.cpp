#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <optional>
#include <source_location>
#include <span>
#include <string>

namespace cucumber_cpp::library::engine
{
    Step::Step(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, const std::optional<cucumber::messages::pickle_table>& dataTable, const std::optional<cucumber::messages::pickle_doc_string>& docString)
        : ExecutionContext{ broadCaster, context, stepOrHookStarted }
        , dataTable{ dataTable }
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
}
