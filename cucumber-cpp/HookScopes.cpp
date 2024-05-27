
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookRegistry.hpp"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp
{
    BeforeAfterScopeExecuter::BeforeAfterScopeExecuter(Context& context, const std::set<std::string, std::less<>>& tags)
        : context{ context }
        , tags{ tags }
    {
    }

    void BeforeAfterScopeExecuter::ExecuteAll(HookType hook)
    {
        for (const auto& match : HookRegistry::Instance().Query(hook, tags))
            match.factory(context)->Execute();
    }
}
