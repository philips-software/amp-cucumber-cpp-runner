#ifndef ENGINE_EXECUTION_CONTEXT_HPP
#define ENGINE_EXECUTION_CONTEXT_HPP

#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <istream>
#include <optional>
#include <source_location>
#include <string>
#include <variant>

namespace cucumber_cpp::library::engine
{
    using StepSkipped = support::StepSkipped;
    using StepPending = support::StepPending;

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
