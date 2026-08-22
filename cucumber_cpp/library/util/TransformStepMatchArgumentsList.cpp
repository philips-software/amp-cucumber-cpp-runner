#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/TransformArgument.hpp"
#include <ranges>

namespace cucumber_cpp::library::util
{
    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::StepMatchArgumentsList& args)
    {
        auto strings = args.stepMatchArguments | std::views::transform([](const auto& arg)
                                                     {
                                                         return util::ToArgument(*arg);
                                                     });
        return { strings.begin(), strings.end() };
    }
}
