
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/HookType.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include <map>
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
        return {
            .id = hookData.id,
            .name = hookData.name.has_value() ? std::make_optional<std::string>(hookData.name.value()) : std::nullopt,
            .sourceReference = std::make_shared<cucumber::messages::SourceReference>(cucumber::messages::SourceReference{
                .uri = hookData.sourceLocation.file_name(),
                .location = std::make_shared<cucumber::messages::Location>(cucumber::messages::Location{
                    .line = hookData.sourceLocation.line(),
                }),
            }),
            .tagExpression = hookData.expression.has_value() ? std::make_optional<std::string>(hookData.expression.value()) : std::nullopt,
            .type = MapHookType(hookData.type),
        };
    }
}
