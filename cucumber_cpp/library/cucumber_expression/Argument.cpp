#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Group.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "fmt/format.h"
#include <cstddef>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    Argument::Argument(ArgumentGroup group, const ParameterType& parameter)
        : group{ std::move(group) }
        , parameter{ parameter }
    {}

    std::vector<Argument> Argument::BuildArguments(const ArgumentGroup& group, std::span<const ParameterType> parameters)
    {
        if (group.children.size() != parameters.size())
            throw std::runtime_error(fmt::format("Mismatch between number of groups ({}) and parameters ({})", group.children.size(), parameters.size()));

        std::size_t index{ 0 };
        auto converted = parameters | std::views::transform([&group, &index](const ParameterType& parameter) -> Argument
                                          {
                                              return { group.children[index++], parameter };
                                          });

        return { converted.begin(), converted.end() };
    }

    ArgumentGroup Argument::Group() const
    {
        return group;
    }

    std::string Argument::Name() const
    {
        return parameter.name;
    }

}
