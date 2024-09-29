
#include "cucumber_cpp/HookScopes.hpp"
#include "cucumber_cpp/Context.hpp"
#include "cucumber_cpp/HookRegistry.hpp"
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
