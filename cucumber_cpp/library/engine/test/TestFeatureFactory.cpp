#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::engine
{

    struct TestFeatureFactory : testing::Test
    {
        FeatureTreeFactory featureTreeFactory;
    };

    TEST_F(TestFeatureFactory, CreateEmptyFeature)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "Custom\n"
               "Description\n";

        auto feature = featureTreeFactory.Create(tmp.Path(), "");
        EXPECT_THAT(feature->Title(), testing::StrEq("Test feature"));
        EXPECT_THAT(feature->Description(), testing::StrEq("Custom\nDescription"));
        EXPECT_THAT(feature->Tags(), testing::Eq(std::set<std::string, std::less<>>{}));
        EXPECT_THAT(feature->Path(), testing::Eq(tmp.Path()));
        EXPECT_THAT(feature->Line(), testing::Eq(1));
        EXPECT_THAT(feature->Column(), testing::Eq(1));
        EXPECT_THAT(feature->Rules().size(), testing::Eq(0));
        EXPECT_THAT(feature->Scenarios().size(), testing::Eq(0));
    }

    TEST_F(TestFeatureFactory, CreateScenario)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "Custom\n"
               "Description\n"
               "  Scenario: Test scenario\n"
               "  Custom Scenario\n"
               "  Description\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");
        EXPECT_THAT(feature->Rules().size(), testing::Eq(0));
        EXPECT_THAT(feature->Scenarios().size(), testing::Eq(1));

        const auto& scenario = feature->Scenarios().front();
        EXPECT_THAT(scenario->Title(), testing::StrEq("Test scenario"));
        EXPECT_THAT(scenario->Description(), testing::StrEq("  Custom Scenario\n  Description"));
        EXPECT_THAT(scenario->Tags(), testing::Eq(std::set<std::string, std::less<>>{}));
        EXPECT_THAT(scenario->Path(), testing::Eq(tmp.Path()));
        EXPECT_THAT(scenario->Line(), testing::Eq(4));
        EXPECT_THAT(scenario->Column(), testing::Eq(3));
        EXPECT_THAT(scenario->Children().size(), testing::Eq(0));
    }

    TEST_F(TestFeatureFactory, CreateRules)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "Custom\n"
               "Description\n"
               "  Rule: Test rule\n"
               "  Custom Rule\n"
               "  Description1\n"
               "    Scenario: Test scenario1\n"
               "    Custom Scenario\n"
               "    Description\n"
               "  Rule: Test rule\n"
               "  Custom Rule\n"
               "  Description2\n"
               "    Scenario: Test scenario2\n"
               "    Custom Scenario\n"
               "    Description\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");
        EXPECT_THAT(feature->Rules().size(), testing::Eq(2));
        EXPECT_THAT(feature->Scenarios().size(), testing::Eq(0));

        const auto& rule1 = feature->Rules().front();
        EXPECT_THAT(rule1->Title(), testing::StrEq("Test rule"));
        EXPECT_THAT(rule1->Description(), testing::StrEq("  Custom Rule\n  Description1"));
        EXPECT_THAT(rule1->Line(), testing::Eq(4));
        EXPECT_THAT(rule1->Column(), testing::Eq(3));
        EXPECT_THAT(rule1->Scenarios().size(), testing::Eq(1));

        const auto& rule1scenario1 = rule1->Scenarios().front();
        EXPECT_THAT(rule1scenario1->Title(), testing::StrEq("Test scenario1"));
        EXPECT_THAT(rule1scenario1->Line(), testing::Eq(7));

        const auto& rule2 = feature->Rules().back();
        EXPECT_THAT(rule2->Title(), testing::StrEq("Test rule"));
        EXPECT_THAT(rule2->Description(), testing::StrEq("  Custom Rule\n  Description2"));
        EXPECT_THAT(rule2->Line(), testing::Eq(10));
        EXPECT_THAT(rule2->Column(), testing::Eq(3));
        EXPECT_THAT(rule2->Scenarios().size(), testing::Eq(1));

        const auto& rule2scenario1 = rule2->Scenarios().front();
        EXPECT_THAT(rule2scenario1->Title(), testing::StrEq("Test scenario2"));
        EXPECT_THAT(rule2scenario1->Line(), testing::Eq(13));
    }

    TEST_F(TestFeatureFactory, CreateSteps)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario1\n"
               "    Given I have a step1\n"
               "    When I do something1\n"
               "    Then I expect something1\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario2\n"
               "      Given I have a step2\n"
               "      When I do something2\n"
               "      Then I expect something2\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario3\n"
               "      Given I have a step3\n"
               "      When I do something3\n"
               "      Then I expect something3\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");
        EXPECT_THAT(feature->Rules().size(), testing::Eq(2));
        EXPECT_THAT(feature->Scenarios().size(), testing::Eq(1));

        const auto& scenario1 = feature->Scenarios()[0];
        EXPECT_THAT(scenario1->Title(), testing::StrEq("Test scenario1"));
        EXPECT_THAT(scenario1->Children().size(), testing::Eq(3));
        EXPECT_THAT(scenario1->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario1->Children()[1]->Type(), testing::Eq(StepType::when));
        EXPECT_THAT(scenario1->Children()[2]->Type(), testing::Eq(StepType::then));
        EXPECT_THAT(scenario1->Children()[0]->Text(), testing::StrEq("I have a step1"));
        EXPECT_THAT(scenario1->Children()[1]->Text(), testing::StrEq("I do something1"));
        EXPECT_THAT(scenario1->Children()[2]->Text(), testing::StrEq("I expect something1"));

        const auto& scenario2 = feature->Rules()[0]->Scenarios()[0];
        EXPECT_THAT(scenario2->Title(), testing::StrEq("Test scenario2"));
        EXPECT_THAT(scenario2->Children().size(), testing::Eq(3));
        EXPECT_THAT(scenario2->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario2->Children()[1]->Type(), testing::Eq(StepType::when));
        EXPECT_THAT(scenario2->Children()[2]->Type(), testing::Eq(StepType::then));
        EXPECT_THAT(scenario2->Children()[0]->Text(), testing::StrEq("I have a step2"));
        EXPECT_THAT(scenario2->Children()[1]->Text(), testing::StrEq("I do something2"));
        EXPECT_THAT(scenario2->Children()[2]->Text(), testing::StrEq("I expect something2"));

        const auto& scenario3 = feature->Rules()[1]->Scenarios()[0];
        EXPECT_THAT(scenario3->Title(), testing::StrEq("Test scenario3"));
        EXPECT_THAT(scenario3->Children().size(), testing::Eq(3));
        EXPECT_THAT(scenario3->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario3->Children()[1]->Type(), testing::Eq(StepType::when));
        EXPECT_THAT(scenario3->Children()[2]->Type(), testing::Eq(StepType::then));
        EXPECT_THAT(scenario3->Children()[0]->Text(), testing::StrEq("I have a step3"));
        EXPECT_THAT(scenario3->Children()[1]->Text(), testing::StrEq("I do something3"));
        EXPECT_THAT(scenario3->Children()[2]->Text(), testing::StrEq("I expect something3"));
    }

    TEST_F(TestFeatureFactory, CreateBackground)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "  Background: Test background\n"
               "    Given a background step1\n"
               "  Scenario: Test scenario1\n"
               "    Given I have a step1\n"
               "  Rule: Test rule\n"
               "    Background: Test background\n"
               "      Given a background step2\n"
               "    Scenario: Test scenario2\n"
               "      Given I have a step2\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario3\n"
               "      Given I have a step3\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");

        const auto& scenario1 = feature->Scenarios()[0];
        EXPECT_THAT(scenario1->Title(), testing::StrEq("Test scenario1"));
        EXPECT_THAT(scenario1->Children().size(), testing::Eq(2));
        EXPECT_THAT(scenario1->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario1->Children()[1]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario1->Children()[0]->Text(), testing::StrEq("a background step1"));
        EXPECT_THAT(scenario1->Children()[1]->Text(), testing::StrEq("I have a step1"));

        const auto& scenario2 = feature->Rules()[0]->Scenarios()[0];
        EXPECT_THAT(scenario2->Title(), testing::StrEq("Test scenario2"));
        EXPECT_THAT(scenario2->Children().size(), testing::Eq(3));
        EXPECT_THAT(scenario2->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario2->Children()[1]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario2->Children()[0]->Text(), testing::StrEq("a background step1"));
        EXPECT_THAT(scenario2->Children()[1]->Text(), testing::StrEq("a background step2"));
        EXPECT_THAT(scenario2->Children()[2]->Text(), testing::StrEq("I have a step2"));

        const auto& scenario3 = feature->Rules()[1]->Scenarios()[0];
        EXPECT_THAT(scenario3->Title(), testing::StrEq("Test scenario3"));
        EXPECT_THAT(scenario3->Children().size(), testing::Eq(2));
        EXPECT_THAT(scenario3->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario3->Children()[1]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario3->Children()[0]->Text(), testing::StrEq("a background step1"));
        EXPECT_THAT(scenario3->Children()[1]->Text(), testing::StrEq("I have a step3"));
    }

    TEST_F(TestFeatureFactory, CreateTagsTags)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "@feature\n"
               "Feature: Test feature\n"
               "  @scenario1\n"
               "  Scenario: Test scenario1\n"
               "    Given I have a step1\n"
               "  @rule\n"
               "  Rule: Test rule\n"
               "    @scenario2\n"
               "    Scenario: Test scenario2\n"
               "      Given I have a step2\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");

        const auto& scenario1 = feature->Scenarios()[0];
        EXPECT_THAT(scenario1->Tags(), testing::Eq(std::set<std::string, std::less<>>{ "@feature", "@scenario1" }));

        const auto& scenario2 = feature->Rules()[0]->Scenarios()[0];
        EXPECT_THAT(scenario2->Tags(), testing::Eq(std::set<std::string, std::less<>>{ "@feature", "@rule", "@scenario2" }));
    }

    TEST_F(TestFeatureFactory, SelectTags)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "@feature\n"
               "Feature: Test feature\n"
               "  Rule: Test rule1\n"
               "  @scenario1 @debug\n"
               "  Scenario: Test scenario1\n"
               "    Given I have a step1\n"
               "  @rule\n"
               "  Rule: Test rule2\n"
               "    @scenario2 @debug\n"
               "    Scenario: Test scenario2\n"
               "      Given I have a step2\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "@debug & @rule");
        EXPECT_THAT(feature->Scenarios().size(), testing::Eq(0));
        EXPECT_THAT(feature->Rules().size(), testing::Eq(1));

        const auto& scenario1 = feature->Rules()[0]->Scenarios()[0];
        EXPECT_THAT(scenario1->Title(), testing::StrEq("Test scenario2"));
    }

    TEST_F(TestFeatureFactory, CreateMultipleScenariosInOneRule)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given I have a step1\n"
               "    Scenario: Test scenario2\n"
               "      Given I have a step2\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");
        EXPECT_THAT(feature->Rules().size(), testing::Eq(1));

        const auto& rule = feature->Rules()[0];
        EXPECT_THAT(rule->Scenarios().size(), testing::Eq(2));

        const auto& scenario1 = rule->Scenarios()[0];
        EXPECT_THAT(scenario1->Title(), testing::StrEq("Test scenario1"));
        EXPECT_THAT(scenario1->Children().size(), testing::Eq(1));
        EXPECT_THAT(scenario1->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario1->Children()[0]->Text(), testing::StrEq("I have a step1"));

        const auto& scenario2 = rule->Scenarios()[1];
        EXPECT_THAT(scenario2->Title(), testing::StrEq("Test scenario2"));
        EXPECT_THAT(scenario2->Children().size(), testing::Eq(1));
        EXPECT_THAT(scenario2->Children()[0]->Type(), testing::Eq(StepType::given));
        EXPECT_THAT(scenario2->Children()[0]->Text(), testing::StrEq("I have a step2"));
    }

    TEST_F(TestFeatureFactory, CreateTable)
    {
        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };

        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario1\n"
               "    Given I have a step1\n"
               "      | a | b |\n"
               "      | c | d |\n";

        const auto feature = featureTreeFactory.Create(tmp.Path(), "");

        const auto& scenario1 = feature->Scenarios()[0];
        EXPECT_THAT(scenario1->Children().size(), testing::Eq(1));
        EXPECT_THAT(scenario1->Children()[0]->Table()[0][0].As<std::string>(), testing::StrEq("a"));
        EXPECT_THAT(scenario1->Children()[0]->Table()[0][1].As<std::string>(), testing::StrEq("b"));
        EXPECT_THAT(scenario1->Children()[0]->Table()[1][0].As<std::string>(), testing::StrEq("c"));
        EXPECT_THAT(scenario1->Children()[0]->Table()[1][1].As<std::string>(), testing::StrEq("d"));
    }
}
