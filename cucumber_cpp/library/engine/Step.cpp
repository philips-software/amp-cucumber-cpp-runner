#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber/messages/doc_string.hpp"
#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include <functional>
#include <optional>
#include <source_location>
#include <span>
#include <string>

namespace cucumber_cpp::library::engine
{
    Step::Step(Context& context, std::optional<std::span<const cucumber::messages::pickle_table_row>> table, const std::optional<cucumber::messages::pickle_doc_string>& docString)
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
