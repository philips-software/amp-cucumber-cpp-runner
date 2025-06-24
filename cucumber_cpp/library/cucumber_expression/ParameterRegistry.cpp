
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include <any>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        template<class T>
        std::function<std::any(MatchRange)> CreateStreamConverter()
        {
            return [](const MatchRange& matches)
            {
                return StringTo<T>(matches.begin()->str());
            };
        }

        std::function<std::any(MatchRange)> CreateStringConverter()
        {
            return [](const MatchRange& matches)
            {
                std::string str = matches[1].matched ? matches[1].str() : matches[3].str();
                str = std::regex_replace(str, std::regex(R"__(\\")__"), "\"");
                str = std::regex_replace(str, std::regex(R"__(\\')__"), "'");
                return str;
            };
        }

        const std::string integerNegativeRegex{ R"__(-?\d+)__" };
        const std::string integerPositiveRegex{ R"__(\d+)__" };
        const std::string floatRegex{ R"__((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)__" };
        const std::string stringDoubleRegex{ R"__("([^\"\\]*(\\.[^\"\\]*)*)")__" };
        const std::string stringSingleRegex{ R"__('([^'\\]*(\\.[^'\\]*)*)')__" };
        const std::string wordRegex{ R"__([^\s]+)__" };
    }

    std::smatch::const_iterator MatchRange::begin() const
    {
        return first;
    }

    std::smatch::const_iterator MatchRange::end() const
    {
        return second;
    }

    const std::ssub_match& MatchRange::operator[](std::size_t index) const
    {
        return *std::next(begin(), index);
    }

    Converter::Converter(std::size_t matches, std::function<std::any(MatchRange)> converter)
        : matches{ matches }
        , converter{ std::move(converter) }
    {}

    ParameterRegistry::ParameterRegistry()
    {
        Add("int", { integerNegativeRegex, integerPositiveRegex }, CreateStreamConverter<std::int32_t>());
        Add("float", { floatRegex }, CreateStreamConverter<float>());
        Add("word", { wordRegex }, CreateStreamConverter<std::string>());
        Add("string", { stringDoubleRegex, stringSingleRegex }, CreateStringConverter());
        Add("", { ".*" }, CreateStreamConverter<std::string>());
        Add("bigdecimal", { floatRegex }, CreateStreamConverter<double>());
        Add("biginteger", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        Add("byte", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int32_t>());
        Add("short", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int32_t>());
        Add("long", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        Add("double", { floatRegex }, CreateStreamConverter<double>());

        // extension
        Add("bool", { wordRegex }, CreateStreamConverter<bool>());
    }

    Parameter ParameterRegistry::Lookup(const std::string& name) const
    {
        if (parameters.contains(name))
            return parameters.at(name);
        return {};
    }

    void ParameterRegistry::Add(std::string name, std::vector<std::string> regex, std::function<std::any(MatchRange)> converter)
    {
        if (parameters.contains(name))
        {
            if (name.empty())
                throw CucumberExpressionError{ "The anonymous parameter type has already been defined" };
            else
                throw CucumberExpressionError{ std::format("There is already a parameter with name {}", name) };
        }

        parameters[name] = Parameter{ name, std::move(regex), std::move(converter) };
    }
}
