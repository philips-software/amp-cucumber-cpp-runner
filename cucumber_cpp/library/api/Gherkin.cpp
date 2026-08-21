#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber/gherkin/App.hpp"
#include "cucumber/gherkin/AstBuilder.hpp"
#include "cucumber/gherkin/Exceptions.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/gherkin/Parser.hpp"
#include "cucumber/gherkin/PickleCompiler.hpp"
#include "cucumber/gherkin/Utils.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/ParseError.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/Source.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
#include <memory>

namespace cucumber_cpp::library::api
{
    std::list<support::PickleSource> CollectPickles(const support::RunOptions::Sources& sources, cucumber::gherkin::IdGeneratorPtr idGenerator, util::Broadcaster& broadcaster)
    {
        std::list<support::PickleSource> pickleSources;

        cucumber::gherkin::Parser<cucumber::gherkin::AstBuilder> parser{ idGenerator };

        for (const auto& path : sources.paths)
        {
            auto source = std::make_shared<cucumber::messages::Source>(cucumber::messages::Source{
                .uri = path.string(),
                .data = cucumber::gherkin::Slurp(path.string()),
            });
            const auto uri = source->uri;

            broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .source = source });

            try
            {
                auto ast = std::make_shared<const cucumber::messages::GherkinDocument>(parser.Parse(uri, source->data));
                broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .gherkinDocument = std::make_shared<cucumber::messages::GherkinDocument>(*ast) });

                cucumber::gherkin::PickleCompiler pc(idGenerator);
                pc.Compile(*ast, uri, [&pickleSources, ast, &broadcaster](const cucumber::messages::Pickle& pickle)
                    {
                        pickleSources.emplace_back(
                            std::make_shared<const cucumber::messages::Pickle>(pickle),
                            ast);

                        broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .pickle = std::make_shared<cucumber::messages::Pickle>(pickle) });
                    });
            }
            catch (const cucumber::gherkin::CompositeParserError& compositeError)
            {
                for (const auto& error : compositeError.Errors())
                {
                    broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .parseError = std::make_shared<cucumber::messages::ParseError>(cucumber::messages::ParseError{
                                                                                 .source = std::make_shared<cucumber::messages::SourceReference>(cucumber::messages::SourceReference{
                                                                                     .uri = uri,
                                                                                     .location = std::make_shared<cucumber::messages::Location>(error->Location()),
                                                                                 }),
                                                                                 .message = error->what(),
                                                                             }) });
                }
            }
            catch (const cucumber::gherkin::ParserError& error)
            {
                broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .parseError = std::make_shared<cucumber::messages::ParseError>(cucumber::messages::ParseError{
                                                                             .source = std::make_shared<cucumber::messages::SourceReference>(cucumber::messages::SourceReference{
                                                                                 .uri = uri,
                                                                                 .location = std::make_shared<cucumber::messages::Location>(error.Location()),
                                                                             }),
                                                                             .message = error.what(),
                                                                         }) });
            }
        }

        return pickleSources;
    }
}
