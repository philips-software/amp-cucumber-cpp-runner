
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/hook_type.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library
{
    namespace
    {
        auto TypeFilter(HookType hookType)
        {
            return [hookType](const auto& keyValue)
            {
                return keyValue.second.type == hookType;
            };
        };

        auto Matches(std::span<const cucumber::messages::pickle_tag> tags)
        {
            return [tags](const auto& keyValue)
            {
                return keyValue.second.tagExpression->Evaluate(tags);
            };
        }

        auto Matches(std::span<const cucumber::messages::tag> tags)
        {
            return [tags](const auto& keyValue)
            {
                return keyValue.second.tagExpression->Evaluate(tags);
            };
        }

        std::map<HookType, cucumber::messages::hook_type> HookTypeMap{
            { HookType::beforeAll, cucumber::messages::hook_type::BEFORE_TEST_RUN },
            { HookType::afterAll, cucumber::messages::hook_type::AFTER_TEST_RUN },
            // { HookType::beforeFeature, cucumber::messages::hook_type::BEFORE_TEST_CASE },
            // { HookType::afterFeature, cucumber::messages::hook_type::AFTER_TEST_CASE },
            { HookType::before, cucumber::messages::hook_type::BEFORE_TEST_CASE },
            { HookType::after, cucumber::messages::hook_type::AFTER_TEST_CASE },
            { HookType::beforeStep, cucumber::messages::hook_type::BEFORE_TEST_STEP },
            { HookType::afterStep, cucumber::messages::hook_type::AFTER_TEST_STEP },
        };
    }

    HookBase::HookBase(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted)
        : engine::ExecutionContext{ broadCaster, context, std::move(stepOrHookStarted) }
    {}

    HookRegistry::Definition::Definition(std::string id, HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, HookFactory factory, std::source_location sourceLocation)
        : type{ type }
        , tagExpression{ tag_expression::Parse(expression.value_or("")) }
        , factory{ factory }
        , hook{
            .id = std::move(id),
            .name = name.has_value() ? std::make_optional<std::string>(name.value()) : std::nullopt,
            .source_reference = cucumber::messages::source_reference{
                .uri = sourceLocation.file_name(),
                .location = cucumber::messages::location{
                    .line = sourceLocation.line(),
                },
            },
            .tag_expression = expression.has_value() ? std::make_optional<std::string>(expression.value()) : std::nullopt,
            .type = HookTypeMap.contains(type) ? std::make_optional(HookTypeMap.at(type)) : std::nullopt,
        }
    {}

    HookRegistry::HookRegistry(cucumber::gherkin::id_generator_ptr idGenerator)
        : idGenerator{ std::move(idGenerator) }
    {
    }

    void HookRegistry::LoadHooks()
    {
        for (const auto& matcher : support::DefinitionRegistration::Instance().GetHooks())
            Register(matcher.id, matcher.type, matcher.expression, matcher.name, matcher.factory, matcher.sourceLocation);
    }

    std::vector<std::string> HookRegistry::FindIds(HookType hookType, std::span<const cucumber::messages::pickle_tag> tags) const
    {
        auto ids = registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)) | std::views::keys;
        return { ids.begin(), ids.end() };
    }

    std::vector<std::string> HookRegistry::FindIds(HookType hookType, std::span<const cucumber::messages::tag> tags) const
    {
        auto ids = registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)) | std::views::keys;
        return { ids.begin(), ids.end() };
    }

    std::size_t HookRegistry::Size() const
    {
        return registry.size();
    }

    std::size_t HookRegistry::Size(HookType hookType) const
    {
        return std::ranges::count(registry | std::views::values, hookType, &Definition::type);
    }

    HookFactory HookRegistry::GetFactoryById(const std::string& id) const
    {
        return registry.at(id).factory;
    }

    const HookRegistry::Definition& HookRegistry::GetDefinitionById(const std::string& id) const
    {
        return registry.at(id);
    }

    void HookRegistry::Register(std::string id, HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, HookFactory factory, std::source_location sourceLocation)
    {
        registry.try_emplace(id, Definition{ id, type, expression, name, factory, sourceLocation });
    }
}
