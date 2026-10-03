
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/HookType.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::optional<cucumber::messages::HookType> MapHookType(HookType hookType)
        {
            switch (hookType)
            {
                case HookType::beforeAll:
                    return cucumber::messages::HookType::BEFORE_TEST_RUN;
                case HookType::afterAll:
                    return cucumber::messages::HookType::AFTER_TEST_RUN;
                case HookType::beforeFeature:
                case HookType::afterFeature:
                    return std::nullopt;
                case HookType::before:
                    return cucumber::messages::HookType::BEFORE_TEST_CASE;
                case HookType::after:
                    return cucumber::messages::HookType::AFTER_TEST_CASE;
                case HookType::beforeStep:
                    return cucumber::messages::HookType::BEFORE_TEST_STEP;
                case HookType::afterStep:
                    return cucumber::messages::HookType::AFTER_TEST_STEP;
            }

            return std::nullopt;
        }
    }

    cucumber::messages::Hook TransformHookData(const HookData& hookData)
    {
        cucumber::messages::Location location;
        location.line = hookData.sourceLocation.line();

        cucumber::messages::SourceReference sourceReference;
        sourceReference.uri = hookData.sourceLocation.file_name();
        sourceReference.location = location;

        cucumber::messages::Hook hook;
        hook.id = hookData.id;
        if (hookData.name.has_value())
            hook.name = hookData.name.value();
        hook.sourceReference = sourceReference;
        if (hookData.expression.has_value())
            hook.tagExpression = hookData.expression.value();
        hook.type = MapHookType(hookData.type);
        return hook;
    }
}
