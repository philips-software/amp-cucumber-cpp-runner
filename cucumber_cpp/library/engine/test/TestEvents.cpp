#include "cucumber_cpp/library/engine/Events.hpp"
#include "cucumber_cpp/library/engine/Observer.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>

namespace cucumber_cpp::library::engine
{
    struct FeatureEventsMock : FeatureEvents
    {
        using FeatureEvents::FeatureEvents;

        MOCK_METHOD(void, FeatureStarted, (), (override));
        MOCK_METHOD(void, FeatureFinished, (), (override));
    };

    struct ScenarioEventsMock : ScenarioEvents
    {
        using ScenarioEvents::ScenarioEvents;

        MOCK_METHOD(void, ScenarioStarted, (), (override));
        MOCK_METHOD(void, ScenarioFinished, (), (override));
    };

    struct StepEventsMock : StepEvents
    {
        using StepEvents::StepEvents;

        MOCK_METHOD(void, StepStarted, (), (override));
        MOCK_METHOD(void, StepFinished, (), (override));

        MOCK_METHOD(void, StepFailure, (), (override));
        MOCK_METHOD(void, StepError, (), (override));
    };

    struct TestEvents : testing::Test
    {
        EventSubjects eventSubjects;

        FeatureEventsMock featureEvents{ eventSubjects };
        ScenarioEventsMock scenarioEvents{ eventSubjects };
        StepEventsMock stepEvents{ eventSubjects };
    };

    TEST_F(TestEvents, Construct)
    {
    }

    TEST_F(TestEvents, NotifyFeatureStarted)
    {
        EXPECT_CALL(featureEvents, FeatureStarted());
        eventSubjects.NotifyFeatureStarted();
    }

    TEST_F(TestEvents, NotifyFeatureFinished)
    {
        EXPECT_CALL(featureEvents, FeatureFinished());
        eventSubjects.NotifyFeatureFinished();
    }

    TEST_F(TestEvents, NotifyScenarioStarted)
    {
        EXPECT_CALL(scenarioEvents, ScenarioStarted());
        eventSubjects.NotifyScenarioStarted();
    }

    TEST_F(TestEvents, NotifyScenarioFinished)
    {
        EXPECT_CALL(scenarioEvents, ScenarioFinished());
        eventSubjects.NotifyScenarioFinished();
    }

    TEST_F(TestEvents, NotifyStepStarted)
    {
        EXPECT_CALL(stepEvents, StepStarted());
        eventSubjects.NotifyStepStarted();
    }

    TEST_F(TestEvents, NotifyStepFinished)
    {
        EXPECT_CALL(stepEvents, StepFinished());
        eventSubjects.NotifyStepFinished();
    }
}
