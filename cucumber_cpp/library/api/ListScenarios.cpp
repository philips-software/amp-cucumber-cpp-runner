#include "cucumber_cpp/library/api/ListScenarios.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/api/LoadPickles.hpp"
#include "cucumber_cpp/library/api/MakeUniqueTestNames.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "fmt/format.h"
#include <fmt/core.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        std::string FormatScenario(const std::string& testName, const cucumber::messages::Pickle& pickle)
        {
            if (pickle.uri.empty())
                return testName;

            if (pickle.location.has_value())
                return fmt::format("{}\t{}:{}", testName, pickle.uri, pickle.location->line);

            return fmt::format("{}\t{}", testName, pickle.uri);
        }
    }

    std::optional<std::vector<std::string>> ListScenarios(const support::RunOptions::Sources& sources, util::Broadcaster& broadcaster)
    {
        cucumber::gherkin::IdGeneratorPtr idGenerator = std::make_shared<cucumber::gherkin::IdGenerator>();
        cucumber::query::Query query;
        util::Listener queryListener{ broadcaster, [&query](const cucumber::messages::Envelope& envelope)
            {
                query.Update(envelope);
            } };

        const auto pickleSources = LoadPickles(sources, idGenerator, broadcaster);
        if (!pickleSources)
            return std::nullopt;

        const auto uniqueNames = MakeUniqueTestNames(query, *pickleSources);

        std::vector<std::string> scenarios;
        for (const auto& pickleSource : *pickleSources)
            scenarios.push_back(FormatScenario(uniqueNames.at(pickleSource.pickle->id), *pickleSource.pickle));

        return scenarios;
    }
}
