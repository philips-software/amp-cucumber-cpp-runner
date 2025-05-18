#ifndef CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP
#define CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP

#include <algorithm>
#include <any>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    using namespace std::literals;

    template<class To>
    inline To StringTo(std::string s)
    {
        if (s.empty())
            return {};

        std::istringstream stream{ std::move(s) };
        To to{};
        stream >> to;
        if (stream.fail())
            throw std::runtime_error{ "Cannnot convert parameter" };

        return to;
    }

    template<>
    inline std::string StringTo<std::string>(std::string s)
    {
        return std::move(s);
    }

    template<>
    inline float StringTo<float>(std::string s)
    {
        return std::atof(s.c_str());
    }

    template<>
    inline double StringTo<double>(std::string s)
    {
        return std::atof(s.c_str());
    }

    namespace details
    {
        inline bool ichar_equals(char a, char b)
        {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        }

        inline bool iequals(std::string_view lhs, std::string_view rhs)
        {
            return std::ranges::equal(lhs, rhs, ichar_equals);
        }
    }

    template<>
    inline bool StringTo<bool>(std::string s)
    {
        using details::iequals;

        return iequals(s, "true") || iequals(s, "1") || iequals(s, "yes") || iequals(s, "on") || iequals(s, "enabled") || iequals(s, "active");
    }

    struct MatchRange : std::pair<std::smatch::const_iterator, std::smatch::const_iterator>
    {
        using std::pair<std::smatch::const_iterator, std::smatch::const_iterator>::pair;

        std::smatch::const_iterator begin() const;
        std::smatch::const_iterator end() const;

        const std::ssub_match& operator[](std::size_t index) const;
    };

    struct Converter
    {
        Converter(std::size_t matches,
            std::function<std::any(MatchRange)> converter);

        std::size_t matches;
        std::function<std::any(MatchRange)> converter;
    };

    struct Parameter
    {
        std::string name;
        std::vector<std::string> regex;
        std::function<std::any(MatchRange)> converter;
    };

    struct ParameterRegistry
    {
        ParameterRegistry();

        Parameter Lookup(const std::string& name) const;
        void AddParameter(std::string name, std::vector<std::string> regex, std::function<std::any(MatchRange)> converter);

    private:
        std::map<std::string, Parameter> parameters{};
    };
}

#endif
