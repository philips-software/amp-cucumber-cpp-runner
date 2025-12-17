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
            Definition(std::string id, HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation);

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
        void Register(std::string id, HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation);

        cucumber::gherkin::id_generator_ptr idGenerator;
        std::map<std::string, Definition> registry;
    };

    struct GlobalHook
    {
        std::string_view name{ "anonymous" };
        std::int32_t order{ 0 };
    };

    struct Hook
    {
        std::string_view tagExpression{ "" };
        std::string_view name{ "anonymous" };
        std::int32_t order{ 0 };
    };

    struct HookRegistration
    {
    private:
        HookRegistration() = default;

    public:
        static inline HookRegistration& Instance()
        {
            static HookRegistration instance;
            return instance;
        }

        struct Entry
        {
            Entry(HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation)
                : type(type)
                , expression{ expression }
                , factory(factory)
                , sourceLocation{ sourceLocation }
            {}

            HookType type;
            std::string_view expression;
            HookFactory factory;
            std::source_location sourceLocation;
            std::string id{ "unassigned" };
        };

        template<class T>
        static std::size_t Register(std::string_view tagExpression, HookType hookType, std::source_location sourceLocation = std::source_location::current());
        template<class T>
        static std::size_t Register(Hook hook, HookType hookType, std::source_location sourceLocation = std::source_location::current());
        template<class T>
        static std::size_t Register(GlobalHook hook, HookType hookType, std::source_location sourceLocation = std::source_location::current());

        std::span<Entry> GetEntries();
        [[nodiscard]] std::span<const Entry> GetEntries() const;

    private:
        std::vector<Entry> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t HookRegistration::Register(std::string_view tagExpression, HookType hookType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(hookType, tagExpression, HookBodyFactory<T>, sourceLocation);
        return Instance().registry.size();
    }

    template<class T>
    std::size_t HookRegistration::Register(Hook hook, HookType hookType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(hookType, hook.tagExpression, HookBodyFactory<T>, sourceLocation);
        return Instance().registry.size();
    }

    template<class T>
    std::size_t HookRegistration::Register(GlobalHook hook, HookType hookType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(hookType, "", HookBodyFactory<T>, sourceLocation);
        return Instance().registry.size();
    }
}

#endif
