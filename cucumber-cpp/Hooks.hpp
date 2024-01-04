#ifndef CUCUMBER_CPP_HOOKS_HPP
#define CUCUMBER_CPP_HOOKS_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/TagExpression.hpp"
#include "nlohmann/json_fwd.hpp"
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    struct HookBeforeAll
    {
        virtual ~HookBeforeAll() = default;
        virtual void Run(Context& context) const = 0;
    };

    struct HookAfterAll
    {
        virtual ~HookAfterAll() = default;
        virtual void Run(Context& context) const = 0;
    };

    struct ConditionalHook
    {
        virtual ~ConditionalHook() = default;

    protected:
        ConditionalHook() = default;

    public:
        void Run(Context& context, const nlohmann::json& scenarioTags) const
        {
            if (ShouldRun(scenarioTags))
            {
                Run(context);
            }
        }

    private:
        bool ShouldRun(const nlohmann::json& scenarioTags) const
        {
            return IsTagExprSelected(Tags(), scenarioTags);
        }

        virtual std::string Tags() const = 0;
        virtual void Run(Context& context) const = 0;
    };

    struct HookBeforeFeature : ConditionalHook
    {};

    struct HookAfterFeature : ConditionalHook
    {};

    struct HookBefore : ConditionalHook
    {
    };

    // struct HookAround tbd
    // {
    //     virtual ~HookAround() = default;
    //     virtual void Run(Context& context) const = 0;
    //     virtual const std::string& Tags() const = 0;
    // };

    struct HookAfter : ConditionalHook
    {
    };

    struct HookBeforeStep : ConditionalHook
    {
    };

    struct HookAfterStep : ConditionalHook
    {
    };

    struct Hooks
    {
        explicit Hooks();

        void BeforeAll(Context& context);
        void AfterAll(Context& context);

        void BeforeFeature(Context& context, const nlohmann::json& json);
        void AfterFeature(Context& context, const nlohmann::json& json);

        void Before(Context& context, const nlohmann::json& json);
        void After(Context& context, const nlohmann::json& json);

        void BeforeStep(Context& context, const nlohmann::json& json);
        void AfterStep(Context& context, const nlohmann::json& json);

        void Register(std::unique_ptr<HookBeforeAll> hook);
        void Register(std::unique_ptr<HookAfterAll> hook);
        void Register(std::unique_ptr<HookBeforeFeature>);
        void Register(std::unique_ptr<HookAfterFeature>);
        void Register(std::unique_ptr<HookBefore> hook);
        void Register(std::unique_ptr<HookAfter> hook);
        void Register(std::unique_ptr<HookBeforeStep> hook);
        void Register(std::unique_ptr<HookAfterStep> hook);

    private:
        std::vector<std::unique_ptr<HookBeforeAll>> hooksBeforeAll;
        std::vector<std::unique_ptr<HookAfterAll>> hooksAfterAll;
        std::vector<std::unique_ptr<HookBeforeFeature>> hooksBeforeFeature;
        std::vector<std::unique_ptr<HookAfterFeature>> hooksAfterFeature;
        std::vector<std::unique_ptr<HookBefore>> hooksBefore;
        std::vector<std::unique_ptr<HookAfter>> hooksAfter;
        std::vector<std::unique_ptr<HookBeforeStep>> hooksBeforeStep;
        std::vector<std::unique_ptr<HookAfterStep>> hooksAfterStep;
    };

    template<class T>
    struct HookRegistration
    {
        static bool Register(std::unique_ptr<T> hook)
        {
            Instance().hooks.push_back(std::move(hook));
            return true;
        }

        static void RegisterAll(Hooks& hookRepository)
        {
            auto& hooks = Instance().hooks;

            for (auto i = 0; i < hooks.size(); ++i)
            {
                hookRepository.Register(std::move(hooks[i]));
            }

            hooks.clear();
        }

    private:
        static HookRegistration<T>& Instance()
        {
            static HookRegistration<T> instance;
            return instance;
        }

        std::vector<std::unique_ptr<T>> hooks;
    };

    struct BeforeAfterAllScope
    {
        BeforeAfterAllScope(Hooks& hooks, Context& context)
            : hooks{ hooks }
            , context{ context }
        {
            hooks.BeforeAll(context);
        }

        ~BeforeAfterAllScope()
        {
            hooks.AfterAll(context);
        }

    private:
        Hooks& hooks;
        Context& context;
    };

    struct BeforeAfterFeatureHookScope
    {
        BeforeAfterFeatureHookScope(Hooks& hooks, Context& context, const nlohmann::json& json)
            : hooks{ hooks }
            , context{ context }
            , json{ json }
        {
            hooks.BeforeFeature(context, json);
        }

        ~BeforeAfterFeatureHookScope()
        {
            hooks.AfterFeature(context, json);
        }

    private:
        Hooks& hooks;
        Context& context;
        const nlohmann::json& json;
    };

    struct BeforeAfterHookScope
    {
        BeforeAfterHookScope(Hooks& hooks, Context& context, const nlohmann::json& json)
            : hooks{ hooks }
            , context{ context }
            , json{ json }
        {
            hooks.Before(context, json);
        }

        ~BeforeAfterHookScope()
        {
            hooks.After(context, json);
        }

    private:
        Hooks& hooks;
        Context& context;
        const nlohmann::json& json;
    };

    struct BeforeAfterStepHookScope
    {
        BeforeAfterStepHookScope(Hooks& hooks, Context& context, const nlohmann::json& json)
            : hooks{ hooks }
            , context{ context }
            , json{ json }
        {
            hooks.BeforeStep(context, json);
        }

        ~BeforeAfterStepHookScope()
        {
            hooks.AfterStep(context, json);
        }

    private:
        Hooks& hooks;
        Context& context;
        const nlohmann::json& json;
    };
}

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define HOOK_(base)                                                                                                                                                         \
    namespace                                                                                                                                                               \
    {                                                                                                                                                                       \
        struct CONCAT(base, __LINE__)                                                                                                                                       \
            : cucumber_cpp::base                                                                                                                                            \
        {                                                                                                                                                                   \
            void Run(cucumber_cpp::Context& context) const override;                                                                                                        \
        };                                                                                                                                                                  \
        const bool CONCAT(CONCAT(base, __LINE__),IsRegistered){ cucumber_cpp::HookRegistration<cucumber_cpp::base>::Register(std::make_unique<CONCAT(base, __LINE__)>()) }; \
    }                                                                                                                                                                       \
    void CONCAT(base, __LINE__)::Run([[maybe_unused]] cucumber_cpp::Context& context) const

#define HOOK_BEFORE_ALL() HOOK_(HookBeforeAll)
#define HOOK_AFTER_ALL() HOOK_(HookAfterAll)

#define HOOK_COND_(base, cond)                                                                                                                                              \
    namespace                                                                                                                                                               \
    {                                                                                                                                                                       \
        struct CONCAT(base, __LINE__)                                                                                                                                       \
            : cucumber_cpp::base                                                                                                                                            \
        {                                                                                                                                                                   \
            void Run(cucumber_cpp::Context& context) const override;                                                                                                        \
                                                                                                                                                                            \
            std::string Tags() const override                                                                                                                               \
            {                                                                                                                                                               \
                return cond;                                                                                                                                                \
            }                                                                                                                                                               \
        };                                                                                                                                                                  \
        const bool CONCAT(CONCAT(base, __LINE__),IsRegistered){ cucumber_cpp::HookRegistration<cucumber_cpp::base>::Register(std::make_unique<CONCAT(base, __LINE__)>()) }; \
    }                                                                                                                                                                       \
    void CONCAT(base, __LINE__)::Run([[maybe_unused]] cucumber_cpp::Context& context) const

#define HOOK_BEFORE_FEATURE_COND(cond) HOOK_COND_(HookBeforeFeature, cond)
#define HOOK_BEFORE_FEATURE() HOOK_BEFORE_FEATURE_COND("")

#define HOOK_AFTER_FEATURE_COND(cond) HOOK_COND_(HookAfterFeature, cond)
#define HOOK_AFTER_FEATURE() HOOK_AFTER_FEATURE_COND("")

#define HOOK_BEFORE_COND(cond) HOOK_COND_(HookBefore, cond)
#define HOOK_BEFORE() HOOK_BEFORE_COND("")

#define HOOK_AFTER_COND(cond) HOOK_COND_(HookAfter, cond)
#define HOOK_AFTER() HOOK_AFTER_COND("")

#define HOOK_BEFORE_STEP_COND(cond) HOOK_COND_(HookBeforeStep, cond)
#define HOOK_BEFORE_STEP() HOOK_BEFORE_STEP_COND("")

#define HOOK_AFTER_STEP_COND(cond) HOOK_COND_(HookAfterStep, cond)
#define HOOK_AFTER_STEP() HOOK_AFTER_STEP_COND("")

#endif
