#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/api/LoadPickles.hpp"
#include "cucumber_cpp/library/api/MakeUniqueTestNames.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        struct TestMakeUniqueTestNames : testing::Test
        {
            std::list<support::PickleSource> Load()
            {
                support::RunOptions::Sources sources;
                sources.paths.insert(featureFile.Path());

                auto pickleSources = LoadPickles(sources, std::make_shared<cucumber::gherkin::IdGenerator>(), broadcaster);
                return pickleSources.value_or(std::list<support::PickleSource>{});
            }

            std::vector<std::string> NamesInOrder(const std::list<support::PickleSource>& pickleSources) const
            {
                const auto uniqueNames = MakeUniqueTestNames(query, pickleSources);

                std::vector<std::string> names;
                for (const auto& pickleSource : pickleSources)
                    names.push_back(uniqueNames.at(pickleSource.pickle->id));

                return names;
            }

            engine::test_helper::TemporaryFile featureFile{ "TestMakeUniqueTestNames.feature" };
            util::Broadcaster broadcaster;
            cucumber::query::Query query;
            util::Listener queryListener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
                {
                    query.Update(envelope);
                } };
        };
    }

    TEST_F(TestMakeUniqueTestNames, KeepsUniqueNames)
    {
        featureFile << "Feature: F\n"
                       "  Scenario: A\n"
                       "    Given a step\n"
                       "  Scenario: B\n"
                       "    Given a step\n";

        const auto pickleSources = Load();

        EXPECT_THAT(NamesInOrder(pickleSources), testing::ElementsAre("F.A", "F.B"));
    }

    TEST_F(TestMakeUniqueTestNames, SuffixesDuplicatesInOrder)
    {
        featureFile << "Feature: F\n"
                       "  Scenario: S\n"
                       "    Given a step\n"
                       "  Scenario: S\n"
                       "    Given a step\n"
                       "  Scenario: S\n"
                       "    Given a step\n";

        const auto pickleSources = Load();

        EXPECT_THAT(NamesInOrder(pickleSources), testing::ElementsAre("F.S", "F.S.#2", "F.S.#3"));
    }

    TEST_F(TestMakeUniqueTestNames, SkipsSuffixesAlreadyInUse)
    {
        featureFile << "Feature: F\n"
                       "  Scenario: S\n"
                       "    Given a step\n"
                       "  Scenario: S.#2\n"
                       "    Given a step\n"
                       "  Scenario: S\n"
                       "    Given a step\n";

        const auto pickleSources = Load();

        EXPECT_THAT(NamesInOrder(pickleSources), testing::ElementsAre("F.S", "F.S.#2", "F.S.#3"));
    }

    TEST_F(TestMakeUniqueTestNames, NumbersDuplicatesInGivenOrder)
    {
        featureFile << "Feature: F\n"
                       "  Scenario: S\n"
                       "    Given a step\n"
                       "  Scenario: S\n"
                       "    Given a step\n";

        auto pickleSources = Load();
        const auto firstId = pickleSources.front().pickle->id;
        pickleSources.reverse();

        const auto uniqueNames = MakeUniqueTestNames(query, pickleSources);

        EXPECT_THAT(uniqueNames.at(firstId), testing::StrEq("F.S.#2"));
    }
}
