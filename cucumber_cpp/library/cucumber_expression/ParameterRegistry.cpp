
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
            return [](MatchRange matches)
            {
                return StringTo<T>(matches.begin()->str());
            };
        }

        std::function<std::any(MatchRange)> CreateStringConverter()
        {
            return [](MatchRange matches)
            {
                std::string str{ StringTo<std::string>(matches.begin()->str()) };
                str.erase(0, 1);
                str.erase(str.size() - 1, 1);
                return str;
            };
        }
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

    ParameterRegistry::ParameterRegistry()
    {
        const static std::string integerNegativeRegex{ R"__(-?\d+)__" };
        const static std::string integerPositiveRegex{ R"__(\d+)__" };
        const static std::string floatRegex{ R"__((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)__" };
        const static std::string stringDoubleRegex{ R"__("(?:[^"\\]*(?:\.[^"\\]*)*)")__" };
        const static std::string stringSingleRegex{ R"__('(?:[^'\\]*(?:\.[^'\\]*)*)')__" };
        const static std::string wordRegex{ R"__([^\s]+)__" };

        AddParameter("int", { integerNegativeRegex, integerPositiveRegex }, CreateStreamConverter<std::int64_t>());
        AddParameter("float", { floatRegex }, CreateStreamConverter<float>());
        AddParameter("word", { wordRegex }, CreateStreamConverter<std::string>());
        AddParameter("string", { stringDoubleRegex, stringSingleRegex }, CreateStringConverter());
        AddParameter("", { ".*" }, CreateStreamConverter<std::string>());
        AddParameter("bigdecimal", { floatRegex }, CreateStreamConverter<double>());
        AddParameter("biginteger", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddParameter("byte", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddParameter("short", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddParameter("long", { { integerNegativeRegex, integerPositiveRegex } }, CreateStreamConverter<std::int64_t>());
        AddParameter("double", { floatRegex }, CreateStreamConverter<double>());

        // extension
        AddParameter("bool", { wordRegex }, CreateStreamConverter<bool>());
    }

    Parameter ParameterRegistry::Lookup(const std::string& name) const
    {
        if (parameters.contains(name))
            return parameters.at(name);

        return {};
    }

    void ParameterRegistry::AddParameter(std::string name, std::vector<std::string> regex, std::function<std::any(MatchRange)> converter)
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
