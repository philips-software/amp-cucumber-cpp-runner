#ifndef TEST_HELPER_TESTRUNNERMOCK_HPP
#define TEST_HELPER_TESTRUNNERMOCK_HPP

#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "gmock/gmock.h"
#include <memory>
#include <vector>

namespace cucumber_cpp::library::engine::test_helper
{
    struct TestRunnerMock : TestRunner
    {
        MOCK_METHOD(void, Run, (const std::vector<std::unique_ptr<FeatureInfo>>& feature), (override));
    };
}

#endif
