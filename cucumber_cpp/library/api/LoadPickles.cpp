#include "cucumber_cpp/library/api/LoadPickles.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/ParseError.hpp"
#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        struct ParseErrorListener : util::Listener
        {
            explicit ParseErrorListener(util::Broadcaster& broadcaster)
                : Listener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
                    {
                        if (envelope.parseError)
                            parseErrors.push_back(envelope.parseError.value());
                    } }
            {}

            std::vector<cucumber::messages::ParseError> parseErrors;
        };

        void ReportParseError(const cucumber::messages::ParseError& parseError)
        {
            const auto uri = parseError.source.uri.value_or("unknown source");
            const auto line = parseError.source.location.has_value() ? fmt::format(":{}", parseError.source.location.value().line) : "";
            const auto column = parseError.source.location.has_value() && parseError.source.location.value().column.has_value() ? fmt::format(":{}", parseError.source.location.value().column.value()) : "";

            const auto messageStart = parseError.message.find(": ");
            const auto message = messageStart != std::string::npos ? parseError.message.substr(messageStart + 2) : parseError.message;

            fmt::println(std::cerr, "Parse error in: \"{}{}{}\" {}", uri, line, column, message);
        }
    }

    std::optional<std::list<support::PickleSource>> LoadPickles(const support::RunOptions::Sources& sources, const cucumber::gherkin::IdGeneratorPtr& idGenerator, util::Broadcaster& broadcaster)
    {
        ParseErrorListener parseErrorListener{ broadcaster };
        auto pickleSources = CollectPickles(sources, idGenerator, broadcaster);

        if (!parseErrorListener.parseErrors.empty())
        {
            for (const auto& parseError : parseErrorListener.parseErrors)
                ReportParseError(parseError);

            return std::nullopt;
        }

        if (sources.tagExpression)
            pickleSources.remove_if([&sources](const support::PickleSource& pickleSource)
                {
                    return !sources.tagExpression->Evaluate(util::TransformPickleTags(pickleSource.pickle->tags));
                });

        return pickleSources;
    }
}
