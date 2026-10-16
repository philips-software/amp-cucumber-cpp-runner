#include "cucumber/gherkin/AstBuilder.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/gherkin/Parser.hpp"
#include "cucumber/gherkin/PickleCompiler.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/util/GetHierarchicalTestName.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library::util
{
    namespace
    {
        struct TestGetHierarchicalTestName : testing::Test
        {
            std::vector<cucumber::messages::Pickle> Compile(const std::string& data)
            {
                const std::string uri{ "test.feature" };
                const auto idGenerator = std::make_shared<cucumber::gherkin::IdGenerator>();
                cucumber::gherkin::Parser<cucumber::gherkin::AstBuilder> parser{ idGenerator };
                const auto gherkinDocument = parser.Parse(uri, data);

                envelopes.emplace_back().gherkinDocument = gherkinDocument;
                query.Update(envelopes.back());

                cucumber::gherkin::PickleCompiler pickleCompiler{ idGenerator };
                auto pickles = pickleCompiler.Compile(gherkinDocument, uri);

                for (const auto& pickle : pickles)
                {
                    envelopes.emplace_back().pickle = pickle;
                    query.Update(envelopes.back());
                }

                return pickles;
            }

            // Query refers into the envelopes, so they must outlive it
            std::list<cucumber::messages::Envelope> envelopes;
            cucumber::query::Query query;
        };
    }

    TEST_F(TestGetHierarchicalTestName, CombinesFeatureAndScenarioName)
    {
        const auto pickles = Compile("Feature: My feature\n"
                                     "  Scenario: My scenario\n"
                                     "    Given a step\n");

        ASSERT_THAT(pickles.size(), testing::Eq(1));
        EXPECT_THAT(GetHierarchicalTestName(query, pickles.at(0)), testing::StrEq("My feature.My scenario"));
    }

    TEST_F(TestGetHierarchicalTestName, IncludesRuleName)
    {
        const auto pickles = Compile("Feature: My feature\n"
                                     "  Rule: My rule\n"
                                     "    Scenario: My scenario\n"
                                     "      Given a step\n");

        ASSERT_THAT(pickles.size(), testing::Eq(1));
        EXPECT_THAT(GetHierarchicalTestName(query, pickles.at(0)), testing::StrEq("My feature.My rule.My scenario"));
    }

    TEST_F(TestGetHierarchicalTestName, ReplacesTabsWithSpaces)
    {
        const auto pickles = Compile("Feature: My\tfeature\n"
                                     "  Rule: My\trule\n"
                                     "    Scenario: My\tscenario\n"
                                     "      Given a step\n");

        ASSERT_THAT(pickles.size(), testing::Eq(1));
        ASSERT_THAT(pickles.at(0).name, testing::StrEq("My\tscenario"));
        EXPECT_THAT(GetHierarchicalTestName(query, pickles.at(0)), testing::StrEq("My feature.My rule.My scenario"));
    }

    TEST_F(TestGetHierarchicalTestName, ReplacesTabsWithSpacesWithoutLineage)
    {
        cucumber::messages::Pickle pickle;
        pickle.name = "My\tscenario";
        pickle.astNodeIds = { "unknown" };

        EXPECT_THAT(GetHierarchicalTestName(query, pickle), testing::StrEq("My scenario"));
    }
}
