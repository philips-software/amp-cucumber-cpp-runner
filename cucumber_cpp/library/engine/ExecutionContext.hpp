#ifndef ENGINE_EXECUTION_CONTEXT_HPP
#define ENGINE_EXECUTION_CONTEXT_HPP

#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <exception>
#include <istream>
#include <optional>
#include <source_location>
#include <string>
#include <utility>
#include <variant>

namespace cucumber_cpp::library::engine
{
    struct StepSkipped : std::exception
    {
        StepSkipped(std::string message, std::source_location sourceLocation)
            : message{ std::move(message) }
            , sourceLocation{ sourceLocation }
        {
        }

        std::string message;
        std::source_location sourceLocation;
    };

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

    struct AttachOptions
    {
        std::string mediaType;
        std::optional<std::string> fileName;
    };

    using OptionsOrMediaType = std::variant<std::string, AttachOptions>;
    using StepOrHookStarted = std::variant<cucumber::messages::test_step_started, cucumber::messages::test_run_hook_started>;

    struct ExecutionContext
    {
        ExecutionContext(util::Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted);

    protected:
        void Attach(std::string data, OptionsOrMediaType mediaType);
        void Attach(std::istream& data, OptionsOrMediaType mediaType);
        void Log(std::string text);
        void Link(std::string url, std::optional<std::string> title = std::nullopt);

        [[noreturn]] static void Skipped(const std::string& message = "", std::source_location current = std::source_location::current()) noexcept(false);
        [[noreturn]] static void Pending(const std::string& message = "", std::source_location current = std::source_location::current()) noexcept(false);

        Context& context;

    private:
        void Attach(std::string data, cucumber::messages::attachment_content_encoding encoding, OptionsOrMediaType mediaType);

        util::Broadcaster& broadCaster;
        StepOrHookStarted stepOrHookStarted;
    };
}

#endif
