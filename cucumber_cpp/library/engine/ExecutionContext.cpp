#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "base64.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <istream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <variant>

/*

std::optional<std::string> read_file( std::istream & is ) {
  if( not is.seekg( 0, std::ios_base::seek_dir::end ) ) {
    return std::nullopt;
  }
  auto const sz = is.tellg( );
  if( not is.seekg( 0 ) ) {
    return std::nullopt;
  }
  auto result = std::string( '\0', static_cast<std::size_t>( sz ) );
  if( not is.read( result.data( ), sz ) ) {
    return std::nullopt;
  }
  if( sz != is.gcount( ) ) {
    return std::nullopt;
  }
  return result;
}

*/

namespace cucumber_cpp::library::engine
{
    namespace
    {
        constexpr auto LogMediaType{ "text/x.cucumber.log+plain" };
        constexpr auto LinkMediaType{ "text/uri-list" };

        std::pair<std::optional<std::string>, std::optional<std::string>> ReadTestStepStartedIds(StepOrHookStarted stepOrHookStarted)
        {
            if (std::holds_alternative<cucumber::messages::test_step_started>(stepOrHookStarted))
            {
                return {
                    std::get<cucumber::messages::test_step_started>(stepOrHookStarted).test_case_started_id,
                    std::get<cucumber::messages::test_step_started>(stepOrHookStarted).test_step_id,
                };
            }

            return { std::nullopt, std::nullopt };
        }

        std::optional<std::string> ReadTestRunHookStartedIds(StepOrHookStarted stepOrHookStarted)
        {
            if (std::holds_alternative<cucumber::messages::test_run_hook_started>(stepOrHookStarted))
                return std::get<cucumber::messages::test_run_hook_started>(stepOrHookStarted).id;

            return std::nullopt;
        }
    }

    ExecutionContext::ExecutionContext(util::Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted)
        : context{ context }
        , broadCaster{ broadCaster }
        , stepOrHookStarted{ std::move(stepOrHookStarted) }
    {}

    void ExecutionContext::Attach(std::string data, OptionsOrMediaType mediaType)
    {
        Attach(std::move(data), cucumber::messages::attachment_content_encoding::IDENTITY, std::move(mediaType));
    }

    void ExecutionContext::Attach(std::istream& data, OptionsOrMediaType mediaType)
    {
        std::string buffer{ std::istreambuf_iterator<char>{ data }, std::istreambuf_iterator<char>{} };

        buffer = base64::to_base64(buffer);

        Attach(std::move(buffer), cucumber::messages::attachment_content_encoding::BASE64, mediaType);
    }

    void ExecutionContext::Log(std::string text)
    {
        Attach(std::move(text), std::string{ LogMediaType });
    }

    void ExecutionContext::Link(std::string url, std::optional<std::string> title)
    {
        Attach(std::move(url), AttachOptions{
                                   .mediaType = LinkMediaType,
                                   .fileName = std::move(title),
                               });
    }

    void ExecutionContext::Attach(std::string data, cucumber::messages::attachment_content_encoding encoding, OptionsOrMediaType mediaType)
    {
        const auto options = std::holds_alternative<std::string>(mediaType)
                                 ? AttachOptions{ .mediaType = std::get<std::string>(mediaType) }
                                 : std::get<AttachOptions>(mediaType);

        auto [test_case_started_id, test_step_id] = ReadTestStepStartedIds(stepOrHookStarted);
        auto test_run_hook_started_id = ReadTestRunHookStartedIds(stepOrHookStarted);

        broadCaster.BroadcastEvent({
            .attachment = cucumber::messages::attachment{
                .body = std::move(data),
                .content_encoding = encoding,
                .file_name = std::move(options.fileName),
                .media_type = std::move(options.mediaType),
                .test_case_started_id = std::move(test_case_started_id),
                .test_step_id = std::move(test_step_id),
                .test_run_hook_started_id = std::move(test_run_hook_started_id),
                .timestamp = support::TimestampNow(),
            },
        });
    }
}
