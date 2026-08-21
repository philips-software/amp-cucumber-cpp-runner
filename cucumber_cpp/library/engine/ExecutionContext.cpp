#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "base64.hpp"
#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/AttachmentContentEncoding.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include "cucumber_cpp/library/util/TestRunHookStarted.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <utility>
#include <variant>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        constexpr auto logMediaType{ "text/x.cucumber.log+plain" };
        constexpr auto linkMediaType{ "text/uri-list" };

        std::pair<std::optional<std::string>, std::optional<std::string>> ReadTestStepStartedIds(util::StepOrHookStarted stepOrHookStarted)
        {
            if (std::holds_alternative<util::TestStepStarted>(stepOrHookStarted))
            {
                return {
                    std::get<util::TestStepStarted>(stepOrHookStarted).testCaseStartedId,
                    std::get<util::TestStepStarted>(stepOrHookStarted).testStepId,
                };
            }

            return { std::nullopt, std::nullopt };
        }

        std::optional<std::string> ReadTestRunHookStartedIds(util::StepOrHookStarted stepOrHookStarted)
        {
            if (std::holds_alternative<util::TestRunHookStarted>(stepOrHookStarted))
                return std::get<util::TestRunHookStarted>(stepOrHookStarted).testRunStartedId;

            return std::nullopt;
        }

        void BroadcastAttachment(util::Broadcaster& broadCaster, std::string data, cucumber::messages::AttachmentContentEncoding encoding, OptionsOrMediaType mediaType, const util::StepOrHookStarted& stepOrHookStarted)
        {
            auto options = std::holds_alternative<std::string>(mediaType)
                               ? AttachOptions{ .mediaType = std::get<std::string>(mediaType) }
                               : std::get<AttachOptions>(mediaType);

            auto [testCaseStartedId, testStepId] = ReadTestStepStartedIds(stepOrHookStarted);
            auto testRunHookStartedId = ReadTestRunHookStartedIds(stepOrHookStarted);

            broadCaster.BroadcastEvent(cucumber::messages::Envelope{
                .attachment = std::make_shared<cucumber::messages::Attachment>(cucumber::messages::Attachment{
                    .body = std::move(data),
                    .contentEncoding = encoding,
                    .fileName = std::move(options.fileName),
                    .mediaType = std::move(options.mediaType),
                    .testCaseStartedId = std::move(testCaseStartedId),
                    .testStepId = std::move(testStepId),
                    .testRunHookStartedId = std::move(testRunHookStartedId),
                    .timestamp = std::make_shared<cucumber::messages::Timestamp>(util::TimestampNow()),
                }),
            });
        }
    }

    ExecutionContext::ExecutionContext(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted)
        : context{ context }
        , broadCaster{ broadCaster }
        , stepOrHookStarted{ std::move(stepOrHookStarted) }
    {}

    void ExecutionContext::Attach(std::string data, OptionsOrMediaType mediaType)
    {
        BroadcastAttachment(broadCaster, std::move(data), cucumber::messages::AttachmentContentEncoding::IDENTITY, std::move(mediaType), stepOrHookStarted);
    }

    void ExecutionContext::Attach(std::istream& data, OptionsOrMediaType mediaType)
    {
        std::string buffer{ std::istreambuf_iterator<char>{ data }, std::istreambuf_iterator<char>{} };

        buffer = base64::to_base64(buffer);

        BroadcastAttachment(broadCaster, std::move(buffer), cucumber::messages::AttachmentContentEncoding::BASE64, std::move(mediaType), stepOrHookStarted);
    }

    void ExecutionContext::Log(std::string text)
    {
        Attach(std::move(text), std::string{ logMediaType });
    }

    void ExecutionContext::Link(std::string url, std::optional<std::string> title)
    {
        Attach(std::move(url), AttachOptions{
                                   .mediaType = linkMediaType,
                                   .fileName = std::move(title),
                               });
    }

    void ExecutionContext::Skipped(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepSkipped{ message, current };
    }

    void ExecutionContext::Pending(const std::string& message, std::source_location current) noexcept(false)
    {
        throw StepPending{ message, current };
    }
}
