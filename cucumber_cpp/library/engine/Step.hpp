#ifndef ENGINE_STEP_HPP
#define ENGINE_STEP_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <exception>
#include <source_location>
#include <string>
#include <utility>

namespace cucumber_cpp::library::engine
{
    struct Step
    {
        struct StepPending : std::exception
        {
            StepPending(std::string message, std::source_location sourceLocation)
                : message{ std::move(message) }
                , sourceLocation{ sourceLocation }
            {
            }

            std::string message;
            std::source_location sourceLocation;
        };

        Step(Context& context, const Table& table, const std::string& docString);
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

        [[noreturn]] static void Pending(const std::string& message, std::source_location current = std::source_location::current()) noexcept(false);

        Context& context;
        const Table& table;
        const std::string& docString;
    };
}

#endif
