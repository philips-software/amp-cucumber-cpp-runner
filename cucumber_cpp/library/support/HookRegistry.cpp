
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::support
{
    namespace
    {
        auto TypeFilter(util::HookType hookType)
        {
            return [hookType](const auto& keyValue)
            {
                return keyValue.second.data.type == hookType;
            };
        };

        auto Matches(const std::set<std::string, std::less<>>& tags)
        {
            return [tags](const auto& keyValue)
            {
                return keyValue.second.tagExpression->Evaluate(tags);
            };
        }
    }

    HookRegistry::Definition::Definition(std::string id, util::HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, util::HookFactory factory, std::source_location sourceLocation)
        : data{
            .id = std::move(id),
            .type = type,
            .expression = expression,
            .name = name,
            .sourceLocation = sourceLocation,
        }
        , tagExpression{ tag_expression::Parse(expression.value_or("")) }
        , factory{ factory }
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

    std::vector<std::string> HookRegistry::FindIds(util::HookType hookType, const std::set<std::string, std::less<>>& tags) const
    {
        auto ids = registry | std::views::filter(TypeFilter(hookType)) | std::views::filter(Matches(tags)) | std::views::keys;
        return { ids.begin(), ids.end() };
    }

    [[nodiscard]] std::vector<util::HookData> HookRegistry::HooksByType(util::HookType hookType) const
    {
        auto filtered = registry |
                        std::views::values |
                        std::views::filter([hookType](const Definition& definition)
                            {
                                return definition.data.type == hookType;
                            }) |
                        std::views::transform([](const Definition& definition)
                            {
                                return definition.data;
                            });

        return { filtered.begin(), filtered.end() };
    }

    std::size_t HookRegistry::Size() const
    {
        return registry.size();
    }

    util::HookFactory HookRegistry::GetFactoryById(const std::string& id) const
    {
        return registry.at(id).factory;
    }

    const HookRegistry::Definition& HookRegistry::GetDefinitionById(const std::string& id) const
    {
        return registry.at(id);
    }

    void HookRegistry::Register(const std::string& id, util::HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, util::HookFactory factory, std::source_location sourceLocation)
    {
        registry.try_emplace(id, id, type, expression, name, factory, sourceLocation);
    }
}
