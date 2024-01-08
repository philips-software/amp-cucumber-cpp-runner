
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookRegistry.hpp"
#include "nlohmann/json.hpp"
#include <set>

namespace cucumber_cpp
{
    void BeforeAfterScopeExecuter::ExecuteAll(const std::vector<HookMatch>& matches, Context& context)
    {
        for (const HookMatch& match : matches)
        {
            match.factory(context)->Execute();
        }
    }
}
