#include "cucumber_cpp/library/api/ListScenarios.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "fmt/format.h"
#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        struct TestListScenarios : testing::Test
        {
            std::optional<std::vector<std::string>> List()
            {
                support::RunOptions::Sources sources;
                sources.paths.insert(featureFile.Path());
                return ListScenarios(sources, broadcaster);
            }

            engine::test_helper::TemporaryFile featureFile{ "TestListScenarios.feature" };
            std::string path{ featureFile.Path().string() };
            util::Broadcaster broadcaster;
        };
    }

    TEST_F(TestListScenarios, ListsHierarchicalNameAndLocation)
    {
        featureFile << "Feature: My feature\n"
                       "  Scenario: My scenario\n"
                       "    Given a step\n";

        EXPECT_THAT(List(), testing::Optional(testing::ElementsAre(fmt::format("My feature.My scenario\t{}:2", path))));
    }

    TEST_F(TestListScenarios, DisambiguatesDuplicateNames)
    {
        featureFile << "Feature: My feature\n"
                       "  Scenario: My scenario\n"
                       "    Given a step\n"
                       "  Scenario: My scenario\n"
                       "    Given a step\n"
                       "  Scenario: My scenario\n"
                       "    Given a step\n";

        EXPECT_THAT(List(), testing::Optional(testing::ElementsAre(
                                fmt::format("My feature.My scenario\t{}:2", path),
                                fmt::format("My feature.My scenario.#2\t{}:4", path),
                                fmt::format("My feature.My scenario.#3\t{}:6", path))));
    }

    TEST_F(TestListScenarios, ReplacesTabsInNames)
    {
        featureFile << "Feature: My\tfeature\n"
                       "  Scenario: My\tscenario\n"
                       "    Given a step\n";

        EXPECT_THAT(List(), testing::Optional(testing::ElementsAre(fmt::format("My feature.My scenario\t{}:2", path))));
    }

    TEST_F(TestListScenarios, ReturnsNulloptOnParseError)
    {
        featureFile << "Feature: My feature\n"
                       "  Scenario: My scenario\n"
                       "    Given a step\n"
                       "  This is not gherkin\n";

        testing::internal::CaptureStderr();
        const auto scenarios = List();
        const auto capturedStderr = testing::internal::GetCapturedStderr();

        EXPECT_THAT(scenarios, testing::Eq(std::nullopt));
        EXPECT_THAT(capturedStderr, testing::HasSubstr("Parse error in"));
    }
}
