#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber/gherkin/AstBuilder.hpp"
#include "cucumber/gherkin/Exceptions.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/gherkin/Parser.hpp"
#include "cucumber/gherkin/PickleCompiler.hpp"
#include "cucumber/gherkin/Utils.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/ParseError.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/Source.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/MakeShared.hpp"
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

            broadcaster.BroadcastEvent(source);

            try
            {
                auto ast = std::make_shared<const cucumber::messages::GherkinDocument>(parser.Parse(uri, source->data));
                broadcaster.BroadcastEvent(util::MakeShared(*ast));

                cucumber::gherkin::PickleCompiler pc(idGenerator);
                pc.Compile(*ast, uri, [&pickleSources, ast, &broadcaster](const cucumber::messages::Pickle& pickle)
                    {
                        pickleSources.emplace_back(
                            std::make_shared<const cucumber::messages::Pickle>(pickle),
                            ast);

                        broadcaster.BroadcastEvent(util::MakeShared(pickle));
                    });
            }
            catch (const cucumber::gherkin::CompositeParserError& compositeError)
            {
                for (const auto& error : compositeError.Errors())
                {
                    broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::ParseError{
                        .source = util::MakeShared(cucumber::messages::SourceReference{
                            .uri = uri,
                            .location = util::MakeShared(error->Location()),
                        }),
                        .message = error->what(),
                    }));
                }
            }
            catch (const cucumber::gherkin::ParserError& error)
            {
                broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::ParseError{
                    .source = util::MakeShared(cucumber::messages::SourceReference{
                        .uri = uri,
                        .location = util::MakeShared(error.Location()),
                    }),
                    .message = error.what(),
                }));
            }
        }

        return pickleSources;
    }
}
