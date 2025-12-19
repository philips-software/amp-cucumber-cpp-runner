#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/gherkin/exceptions.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/gherkin/pickle_compiler.hpp"
#include "cucumber/gherkin/utils.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/parse_error.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
#include <memory>

namespace cucumber_cpp::library::api
{
    std::list<support::PickleSource> CollectPickles(const support::RunOptions::Sources& sources, cucumber::gherkin::id_generator_ptr idGenerator, util::Broadcaster& broadcaster)
    {
        std::list<support::PickleSource> pickleSources;

        cucumber::gherkin::parser<> parser{ idGenerator };

        cucumber::messages::envelope envelope;

        for (const auto& path : sources.paths)
        {
            envelope.source = {
                .uri = path.string(),
                .data = cucumber::gherkin::slurp(path.string()),
            };

            broadcaster.BroadcastEvent(envelope);

            try
            {
                auto ast = std::make_shared<const cucumber::messages::gherkin_document>(parser.parse(envelope.source->uri, envelope.source->data));
                broadcaster.BroadcastEvent({ .gherkin_document = *ast });

                cucumber::gherkin::pickle_compiler pc(idGenerator);
                pc.compile(*ast, envelope.source->uri, [&pickleSources, ast, &broadcaster](const auto& pickle)
                    {
                        pickleSources.emplace_back(
                            std::make_shared<const cucumber::messages::pickle>(pickle),
                            ast);

                        broadcaster.BroadcastEvent({ .pickle = pickle });
                    });
            }
            catch (const cucumber::gherkin::composite_parser_error& compositeError)
            {
                for (const auto& error : compositeError.errors())
                {
                    broadcaster.BroadcastEvent({ .parse_error = cucumber::messages::parse_error{
                                                     .source = cucumber::messages::source_reference{
                                                         .uri = envelope.source->uri,
                                                         .location = error->location(),
                                                     },
                                                     .message = error->what(),
                                                 } });
                }
            }
            catch (const cucumber::gherkin::parser_error& error)
            {
                broadcaster.BroadcastEvent({ .parse_error = cucumber::messages::parse_error{
                                                 .source = cucumber::messages::source_reference{
                                                     .uri = envelope.source->uri,
                                                     .location = error.location(),
                                                 },
                                                 .message = error.what(),
                                             } });
            }
        }

        return pickleSources;
    }
}
