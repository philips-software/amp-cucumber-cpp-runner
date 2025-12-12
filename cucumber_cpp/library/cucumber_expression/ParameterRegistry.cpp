
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/MatchRange.hpp"
#include <any>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        template<class T>
        ParameterConversion CreateStreamConverter()
        {
            return { .toStrings = [](const MatchRange& matches) -> cucumber::messages::group
                {
                    return { .value = matches.begin()->str() };
                },
                .toAny = [](const cucumber::messages::group& matches) -> std::any
                {
                    return StringTo<T>(matches.value.value());
                } };
        }

        ParameterConversion CreateStringConverter()
        {
            return { .toStrings = [](const MatchRange& matches) -> cucumber::messages::group
                {
                    std::string str = matches[1].matched ? matches[1].str() : matches[3].str();
                    str = std::regex_replace(str, std::regex(R"__(\\")__"), "\"");
                    str = std::regex_replace(str, std::regex(R"__(\\')__"), "'");
                    return { .value = str };
                },
                .toAny = [](const cucumber::messages::group& matches) -> std::any
                {
                    return matches.value.value();
                } };
        }
    }

    Converter::Converter(std::size_t matches, ParameterConversion converter, std::string name)
        : matches{ matches }
        , converter{ std::move(converter) }
        , name{ std::move(name) }
    {}

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
    }

    const std::map<std::string, const Parameter>& ParameterRegistry::GetParameters() const
    {
        return parametersByName;
    }

    const Parameter& ParameterRegistry::Lookup(const std::string& name) const
    {
        return parametersByName.at(name);
    }

    void ParameterRegistry::AddParameter(std::string name, std::vector<std::string> regex, ParameterConversion converter)
    {
        AddParameter(name, regex, converter, false);
    }

    void ParameterRegistry::AddBuiltinParameter(std::string name, std::vector<std::string> regex, ParameterConversion converter)
    {
        AddParameter(name, regex, converter, true);
    }

    void ParameterRegistry::AddParameter(std::string name, std::vector<std::string> regex, ParameterConversion converter, bool isBuiltin)
    {

        if (parametersByName.contains(name))
        {
            if (name.empty())
                throw CucumberExpressionError{ "The anonymous parameter type has already been defined" };
            else
                throw CucumberExpressionError{ std::format("There is already a parameter with name {}", name) };
        }

        parametersByName.emplace(name, Parameter{ name, regex, converter, isBuiltin });
    }
}
