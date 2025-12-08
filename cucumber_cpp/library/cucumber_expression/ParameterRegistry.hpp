#ifndef CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP
#define CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP

#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/MatchRange.hpp"
#include <algorithm>
#include <any>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    using namespace std::literals;

    struct ConversionError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    template<class To>
    inline To StringTo(const std::string& s)
    {
        if (s.empty())
            return {};

        std::istringstream stream{ s };
        To to{};
        stream >> to;
        if (stream.fail())
            throw ConversionError{ std::format("Cannot convert parameter {} in to {}", s, typeid(To).name()) };

        return to;
    }

    template<>
    inline std::string StringTo<std::string>(const std::string& s)
    {
        return s;
    }

    template<>
    inline int32_t StringTo<std::int32_t>(const std::string& s)
    {
        return std::stoi(s);
    }

    template<>
    inline int64_t StringTo<std::int64_t>(const std::string& s)
    {
        return std::stoll(s);
    }

    template<>
    inline float StringTo<float>(const std::string& s)
    {
        return std::stof(s);
    }

    template<>
    inline double StringTo<double>(const std::string& s)
    {
        return std::stod(s);
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
    inline bool StringTo<bool>(const std::string& s)
    {
        using details::iequals;

        return iequals(s, "true") || iequals(s, "1") || iequals(s, "yes") || iequals(s, "on") || iequals(s, "enabled") || iequals(s, "active");
    }

    struct ParameterConversion
    {
        std::function<cucumber::messages::group(MatchRange)> toStrings;
        std::function<std::any(const cucumber::messages::group&)> toAny;
    };

    struct Converter
    {
        Converter(std::size_t matches, ParameterConversion converter, std::string name);

        std::string name;
        std::size_t matches;
        ParameterConversion converter;
    };

    struct Parameter
    {
        std::string name;
        std::vector<std::string> regex;
        ParameterConversion converter;
    };

    struct ParameterRegistration
    {
    protected:
        ~ParameterRegistration() = default;

    public:
        virtual void AddParameter(std::string name, std::vector<std::string> regex, ParameterConversion converter) = 0;
    };

    struct ParameterRegistry : ParameterRegistration
    {
        ParameterRegistry();

        virtual ~ParameterRegistry() = default;

        const std::map<std::string, const Parameter>& GetParameters() const;

        const Parameter& Lookup(const std::string& name) const;

        void AddParameter(std::string name, std::vector<std::string> regex, ParameterConversion converter) override;

    private:
        std::map<std::string, const Parameter> parametersByName;
    };
}

#endif
