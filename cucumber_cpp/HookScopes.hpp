#ifndef CUCUMBER_CPP_HOOKSCOPES_HPP
#define CUCUMBER_CPP_HOOKSCOPES_HPP

#include "cucumber_cpp/Context.hpp"
#include "cucumber_cpp/HookRegistry.hpp"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp
{

    struct BeforeAfterScopeExecuter
    {
        BeforeAfterScopeExecuter(Context& context, const std::set<std::string, std::less<>>& tags);

        void ExecuteAll(HookType hook);

    private:
        Context& context;
        std::set<std::string, std::less<>> tags;
    };

    template<HookType BeforeHook, HookType AfterHook>
    struct BeforeAfterScope : BeforeAfterScopeExecuter
    {
    public:
        BeforeAfterScope(Context& context, const std::set<std::string, std::less<>>& tags = {});

        BeforeAfterScope(const BeforeAfterScope&) = delete;
        BeforeAfterScope& operator=(const BeforeAfterScope&) = delete;
        BeforeAfterScope(BeforeAfterScope&&) = delete;
        BeforeAfterScope& operator=(BeforeAfterScope&&) = delete;

    protected:
        ~BeforeAfterScope();
    };

    struct BeforeAfterAllScope : BeforeAfterScope<HookType::beforeAll, HookType::afterAll>
    {
        using BeforeAfterScope::BeforeAfterScope;
    };

    struct BeforeAfterFeatureHookScope : BeforeAfterScope<HookType::beforeFeature, HookType::afterFeature>
    {
        using BeforeAfterScope::BeforeAfterScope;
    };

    struct BeforeAfterHookScope : BeforeAfterScope<HookType::before, HookType::after>
    {
        using BeforeAfterScope::BeforeAfterScope;
    };

    struct BeforeAfterStepHookScope : BeforeAfterScope<HookType::beforeStep, HookType::afterStep>
    {
        using BeforeAfterScope::BeforeAfterScope;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<HookType BeforeHook, HookType AfterHook>
    BeforeAfterScope<BeforeHook, AfterHook>::BeforeAfterScope(Context& context, const std::set<std::string, std::less<>>& tags)
        : BeforeAfterScopeExecuter{ context, tags }
    {
        BeforeAfterScopeExecuter::ExecuteAll(BeforeHook);
    }

    template<HookType BeforeHook, HookType AfterHook>
    BeforeAfterScope<BeforeHook, AfterHook>::~BeforeAfterScope()
    {
        BeforeAfterScopeExecuter::ExecuteAll(AfterHook);
    }
}

#endif
