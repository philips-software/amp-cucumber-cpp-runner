
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/hook_type.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include <map>
#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::optional<cucumber::messages::hook_type> MapHookType(HookType hookType)
        {
            switch (hookType)
            {
                case HookType::beforeAll:
                    return cucumber::messages::hook_type::BEFORE_TEST_RUN;
                case HookType::afterAll:
                    return cucumber::messages::hook_type::AFTER_TEST_RUN;
                case HookType::beforeFeature:
                case HookType::afterFeature:
                    return std::nullopt;
                case HookType::before:
                    return cucumber::messages::hook_type::BEFORE_TEST_CASE;
                case HookType::after:
                    return cucumber::messages::hook_type::AFTER_TEST_CASE;
                case HookType::beforeStep:
                    return cucumber::messages::hook_type::BEFORE_TEST_STEP;
                case HookType::afterStep:
                    return cucumber::messages::hook_type::AFTER_TEST_STEP;
            }

            return std::nullopt;
        }
    }

    cucumber::messages::hook TransformHookData(const HookData& hookData)
    {
        return {
            .id = hookData.id,
            .name = hookData.name.has_value() ? std::make_optional<std::string>(hookData.name.value()) : std::nullopt,
            .source_reference = cucumber::messages::source_reference{
                .uri = hookData.sourceLocation.file_name(),
                .location = cucumber::messages::location{
                    .line = hookData.sourceLocation.line(),
                },
            },
            .tag_expression = hookData.expression.has_value() ? std::make_optional<std::string>(hookData.expression.value()) : std::nullopt,
            .type = MapHookType(hookData.type),
        };
    }
}
