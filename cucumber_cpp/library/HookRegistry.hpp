#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library
{
    enum struct HookType
    {
        beforeAll,
        afterAll,
        beforeFeature,
        afterFeature,
        before,
        after,
        beforeStep,
        afterStep,
    };

    struct HookBase : engine::ExecutionContext
    {
        HookBase(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted);

        virtual ~HookBase() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }
    };

    using HookFactory = std::unique_ptr<Body> (&)(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted);

    template<class T>
    std::unique_ptr<Body> HookBodyFactory(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted)
    {
        return std::make_unique<T>(broadCaster, context, stepOrHookStarted);
    }

    struct HookMatch
    {
        explicit HookMatch(HookFactory factory)
            : factory(factory)
        {}

        HookFactory factory;
    };

    struct HookRegistry
    {
        struct Definition
        {
            Definition(std::string id, HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, HookFactory factory, std::source_location sourceLocation);

            HookType type;
            std::unique_ptr<tag_expression::Expression> tagExpression;
            HookFactory factory;
            cucumber::messages::hook hook;
        };

        explicit HookRegistry(cucumber::gherkin::id_generator_ptr idGenerator);

        void LoadHooks();

        std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::pickle_tag> tags = {}) const;
        std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::tag> tags) const;

        std::vector<cucumber::messages::hook> HooksByType(HookType hookType) const
        {
            auto filtered = registry |
                            std::views::values |
                            std::views::filter([hookType](const Definition& definition)
                                {
                                    return definition.type == hookType;
                                }) |
                            std::views::transform([](const Definition& definition)
                                {
                                    return definition.hook;
                                });

            return { filtered.begin(), filtered.end() };
        }

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(HookType hookType) const;

        HookFactory GetFactoryById(std::string id) const;
        const Definition& GetDefinitionById(std::string id) const;

    private:
        void Register(std::string id, HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, HookFactory factory, std::source_location sourceLocation);

        cucumber::gherkin::id_generator_ptr idGenerator;
        std::map<std::string, Definition> registry;
    };
}

#endif
