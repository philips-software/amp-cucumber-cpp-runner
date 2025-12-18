#ifndef ENGINE_STEP_HPP
#define ENGINE_STEP_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <exception>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <utility>

namespace cucumber_cpp::library::engine
{
    struct Step : ExecutionContext
    {
        Step(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, std::optional<std::span<const cucumber::messages::pickle_table_row>> table, const std::optional<cucumber::messages::pickle_doc_string>& docString);
        virtual ~Step() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }

    protected:
        void Given(const std::string& step) const;
        void When(const std::string& step) const;
        void Then(const std::string& step) const;

        std::optional<std::span<const cucumber::messages::pickle_table_row>> table;
        const std::optional<cucumber::messages::pickle_doc_string>& docString;
    };
}

#endif
