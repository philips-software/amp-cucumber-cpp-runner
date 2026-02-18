#ifndef CUCUMBER_EXPRESSION_ARGUMENT_HPP
#define CUCUMBER_EXPRESSION_ARGUMENT_HPP

#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <span>
#include <string>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct Argument
    {
    private:
        Argument(cucumber::messages::group group, const Parameter& parameter);

    public:
        static std::vector<Argument> BuildArguments(const cucumber::messages::group& group, std::span<const Parameter> parameters);

        template<class T>
        T GetValue() const
        {
            return ConverterTypeMap<T>::Instance().at(parameter.name)(group);
        }

        [[nodiscard]] cucumber::messages::group Group() const;
        [[nodiscard]] std::string Name() const;

    private:
        cucumber::messages::group group;
        const Parameter& parameter;
    };
}

#endif
