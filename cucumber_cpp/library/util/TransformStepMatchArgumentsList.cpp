
#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/TransformArgument.hpp"
#include <ranges>

namespace cucumber_cpp::library::util
{
    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::step_match_arguments_list& args)
    {
        auto strings = args.step_match_arguments | std::views::transform(util::ToArgument);
        return { strings.begin(), strings.end() };
    }
}
