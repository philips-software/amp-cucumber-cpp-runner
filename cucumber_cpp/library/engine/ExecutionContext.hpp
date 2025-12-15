#ifndef ENGINE_EXECUTION_CONTEXT_HPP
#define ENGINE_EXECUTION_CONTEXT_HPP

#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <istream>
#include <optional>
#include <string>
#include <variant>

namespace cucumber_cpp::library::engine
{
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

        Context& context;

    private:
        void Attach(std::string data, cucumber::messages::attachment_content_encoding encoding, OptionsOrMediaType mediaType);

        util::Broadcaster& broadCaster;
        StepOrHookStarted stepOrHookStarted;
    };
}

#endif
