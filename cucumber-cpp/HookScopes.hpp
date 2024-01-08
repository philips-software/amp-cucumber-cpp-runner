#ifndef CUCUMBER_CPP_HOOKSCOPES_HPP
#define CUCUMBER_CPP_HOOKSCOPES_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookRegistry.hpp"
#include "nlohmann/json_fwd.hpp"
#include <set>
#include <string>

namespace cucumber_cpp
{

    struct BeforeAfterScopeExecuter
    {
        void ExecuteAll(const std::vector<HookMatch>& matches, Context& context);
    };

    template<HookType BeforeHook, HookType AfterHook>
    struct BeforeAfterScope : BeforeAfterScopeExecuter
    {
    public:
        BeforeAfterScope(Context& context, const std::set<std::string>& tags = {});

    protected:
        ~BeforeAfterScope();

    private:
        Context& context;
        std::set<std::string> tags;
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
    BeforeAfterScope<BeforeHook, AfterHook>::BeforeAfterScope(Context& context, const std::set<std::string>& tags)
        : context{ context }
        , tags{ tags }
    {
        ExecuteAll(HookRegistry::Instance().Query(BeforeHook, tags), context);
    }

    template<HookType BeforeHook, HookType AfterHook>
    BeforeAfterScope<BeforeHook, AfterHook>::~BeforeAfterScope()
    {
        ExecuteAll(HookRegistry::Instance().Query(AfterHook, tags), context);
    }
}

#endif
