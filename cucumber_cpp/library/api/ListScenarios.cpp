#include "cucumber_cpp/library/api/ListScenarios.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/api/LoadPickles.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetHierarchicalTestName.hpp"
#include "fmt/format.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        std::string MakeUnique(const std::string& testName, std::set<std::string, std::less<>>& seenNames)
        {
            auto uniqueName = testName;
            for (std::size_t counter = 2; seenNames.contains(uniqueName); ++counter)
                uniqueName = fmt::format("{}.#{}", testName, counter);

            seenNames.insert(uniqueName);
            return uniqueName;
        }

        std::string FormatScenario(std::string testName, const cucumber::messages::Pickle& pickle)
        {
            if (pickle.uri.empty())
                return testName;

            if (pickle.location.has_value())
                return fmt::format("{}\t{}:{}", testName, pickle.uri, pickle.location->line);

            return fmt::format("{}\t{}", testName, pickle.uri);
        }
    }

    std::vector<std::string> ListScenarios(const support::RunOptions::Sources& sources, util::Broadcaster& broadcaster)
    {
        cucumber::gherkin::IdGeneratorPtr idGenerator = std::make_shared<cucumber::gherkin::IdGenerator>();
        cucumber::query::Query query;
        util::Listener queryListener{ broadcaster, [&query](const cucumber::messages::Envelope& envelope)
            {
                query.Update(envelope);
            } };

        const auto pickleSources = LoadPickles(sources, idGenerator, broadcaster);
        if (!pickleSources)
            return {};

        std::vector<std::string> scenarios;
        std::set<std::string, std::less<>> seenNames;

        for (const auto& pickleSource : *pickleSources)
            scenarios.push_back(FormatScenario(MakeUnique(util::GetHierarchicalTestName(query, *pickleSource.pickle), seenNames), *pickleSource.pickle));

        return scenarios;
    }
}
