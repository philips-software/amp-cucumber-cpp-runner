#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <cstddef>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    Argument::Argument(cucumber::messages::group group, const Parameter& parameter)
        : group{ group }
        , parameter{ parameter }
    {}

    std::vector<Argument> Argument::BuildArguments(const cucumber::messages::group& group, std::span<const Parameter> parameters)
    {
        if (group.children.size() != parameters.size())
            throw std::runtime_error("Mismatch between number of groups and parameters");

        std::size_t index{ 0 };
        auto converted = parameters | std::views::transform([&group, &index](const Parameter& parameter) -> Argument
                                          {
                                              return { group.children[index++], parameter };
                                          });

        return { converted.begin(), converted.end() };
    }

    cucumber::messages::group Argument::Group() const
    {
        return group;
    }

    std::string Argument::Name() const
    {
        return parameter.name;
    }

}
