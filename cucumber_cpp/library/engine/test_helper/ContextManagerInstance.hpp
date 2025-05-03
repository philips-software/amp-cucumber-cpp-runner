#ifndef TEST_HELPER_CONTEXTMANAGERHELPER_HPP
#define TEST_HELPER_CONTEXTMANAGERHELPER_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <memory>

namespace cucumber_cpp::library::engine::test_helper
{
    struct ContextManagerInstanceStorage
    {
    protected:
        std::shared_ptr<ContextStorageFactoryImpl> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>();
    };

    struct ContextManagerInstance : private ContextManagerInstanceStorage
        , cucumber_cpp::library::engine::ContextManager
    {
        ContextManagerInstance()
            : cucumber_cpp::library::engine::ContextManager{ contextStorageFactory }
        {
        }

    private:
        cucumber_cpp::library::engine::FeatureInfo feature{ {}, {}, {}, {}, {}, {} };
        cucumber_cpp::library::engine::RuleInfo rule{ feature, {}, {}, {}, {}, {} };
        cucumber_cpp::library::engine::ScenarioInfo scenario{ rule, {}, {}, {}, {}, {} };
        cucumber_cpp::library::engine::StepInfo step{ scenario, {}, {}, {}, {}, {} };

        ContextManager::ScopedFeatureContext featureContextScope{ CreateFeatureContext(feature) };
        ContextManager::ScopedRuleContext ruleContextScope{ CreateRuleContext(rule) };
        ContextManager::ScopedScenarioContext scenarioContextScope{ CreateScenarioContext(scenario) };
        ContextManager::ScopedStepContext stepContextScope{ CreateStepContext(step) };
    };
}

#endif
