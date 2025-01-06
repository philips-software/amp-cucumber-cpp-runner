#ifndef TEST_HELPER_TESTRUNNERMOCK_HPP
#define TEST_HELPER_TESTRUNNERMOCK_HPP

#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "gmock/gmock.h"
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library::engine::test_helper
{
    struct TestRunnerMock : TestRunner
    {
        virtual ~TestRunnerMock() = default;

        MOCK_METHOD(void, Run, (const std::vector<std::unique_ptr<FeatureInfo>>& feature), (override));
        MOCK_METHOD(void, NestedStep, (StepType type, std::string step), (override));
    };
}

#endif
