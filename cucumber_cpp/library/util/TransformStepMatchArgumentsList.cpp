
#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/TransformArgument.hpp"
#include <ranges>
#include <string>

namespace cucumber_cpp::library::util
{
    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::step_match_arguments_list& args, const ConverterResolver& resolveConverter)
    {
        auto transform = [&resolveConverter](const auto& arg)
        {
            auto result = ToArgument(arg);
            if (resolveConverter && !result.converterName.empty())
                result.converter = resolveConverter(result.converterName);
            return result;
        };

        auto strings = args.step_match_arguments | std::views::transform(transform);
        return { strings.begin(), strings.end() };
    }
}
