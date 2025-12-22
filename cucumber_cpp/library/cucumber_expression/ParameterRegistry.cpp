
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <map>
#include <optional>
#include <regex>
#include <source_location>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        template<class T>
        std::function<T(const cucumber::messages::group&)> CreateStreamConverter()
        {
            return [](const cucumber::messages::group& matches)
            {
                return StringTo<T>(matches.value.value());
            };
        };
    }

    std::function<std::string(const cucumber::messages::group&)> CreateStringConverter()
    {
        return [](const cucumber::messages::group& matches)
        {
            std::string str = matches.children.front().value.has_value()
                                  ? matches.children.front().value.value()
                                  : matches.children.back().value.value();

            str = std::regex_replace(str, std::regex(R"__(\\")__"), "\"");
            str = std::regex_replace(str, std::regex(R"__(\\')__"), "'");

            return str;
        };
    }

    ParameterRegistry::ParameterRegistry()
    {
        const static std::string integerNegativeRegex{ R"__(-?\d+)__" };
        const static std::string integerPositiveRegex{ R"__(\d+)__" };
        const static std::string floatRegex{ R"__((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)__" };
        const static std::string stringDoubleRegex{ R"__("([^\"\\]*(\\.[^\"\\]*)*)")__" };
        const static std::string stringSingleRegex{ R"__('([^'\\]*(\\.[^'\\]*)*)')__" };
        const static std::string wordRegex{ R"__([^\s]+)__" };

        AddBuiltinParameter("int", { integerNegativeRegex, integerPositiveRegex }, CreateStreamConverter<std::int32_t>());
        AddBuiltinParameter("float", { floatRegex }, CreateStreamConverter<float>());
        AddBuiltinParameter("word", { wordRegex }, CreateStreamConverter<std::string>());
        AddBuiltinParameter("string", { stringDoubleRegex, stringSingleRegex }, CreateStringConverter());
        AddBuiltinParameter("", { ".*" }, CreateStreamConverter<std::string>());
        AddBuiltinParameter("bigdecimal", { floatRegex }, CreateStreamConverter<double>());
        AddBuiltinParameter("biginteger", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddBuiltinParameter("byte", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int32_t>());
        AddBuiltinParameter("short", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int32_t>());
        AddBuiltinParameter("long", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddBuiltinParameter("double", { floatRegex }, CreateStreamConverter<double>());

        // extension
        AddBuiltinParameter("bool", { wordRegex }, CreateStreamConverter<bool>());

        const auto& parameterRegistration = cucumber_cpp::library::ParameterRegistration::Instance();
        for (const auto& parameter : parameterRegistration.GetRegisteredParameters())
            AddParameter(Parameter{ parameter.params.name, { std::string(parameter.params.regex) }, false, parameter.params.useForSnippets, parameter.location });
    }

    const std::map<std::string, const Parameter>& ParameterRegistry::GetParameters() const
    {
        return parametersByName;
    }

    const Parameter& ParameterRegistry::Lookup(const std::string& name) const
    {
        return parametersByName.at(name);
    }

    void ParameterRegistry::AssertParameterIsUnique(const std::string& name) const
    {
        if (parametersByName.contains(name))
        {
            if (name.empty())
                throw CucumberExpressionError{ "The anonymous parameter type has already been defined" };
            else
                throw CucumberExpressionError{ std::format("There is already a parameter with name {}", name) };
        }
    }

    void ParameterRegistry::AddParameter(Parameter parameter)
    {
        AssertParameterIsUnique(parameter.name);

        parametersByName.emplace(parameter.name, parameter);
    }
}
