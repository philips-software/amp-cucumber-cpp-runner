#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/support/Body.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::support
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

        [[nodiscard]] std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::pickle_tag> tags = {}) const;
        [[nodiscard]] std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::tag> tags) const;

        [[nodiscard]] std::vector<cucumber::messages::hook> HooksByType(HookType hookType) const;

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(HookType hookType) const;

        [[nodiscard]] HookFactory GetFactoryById(const std::string& id) const;
        [[nodiscard]] const Definition& GetDefinitionById(const std::string& id) const;

    private:
        void Register(const std::string& id, HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, HookFactory factory, std::source_location sourceLocation);

        cucumber::gherkin::id_generator_ptr idGenerator;
        std::map<std::string, Definition, std::less<>> registry;
    };
}

#endif
