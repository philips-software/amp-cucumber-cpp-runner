#ifndef CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP
#define CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP

#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
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
#include <optional>
#include <source_location>
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

    template<class T>
    using TypeMap = std::map<std::string, std::function<T(const cucumber::messages::group&)>>;

    template<class T>
    struct ConverterTypeMap
    {
        static std::map<std::string, std::function<T(const cucumber::messages::group&)>>& Instance();
    };

    template<class T>
    std::map<std::string, std::function<T(const cucumber::messages::group&)>>& ConverterTypeMap<T>::Instance()
    {
        static std::map<std::string, std::function<T(const cucumber::messages::group&)>> typeMap;
        return typeMap;
    }

    struct Parameter
    {
        std::string name;
        std::vector<std::string> regex;
        bool isBuiltin{ false };
        bool useForSnippets{ false };
        std::source_location location;
    };

    struct ParameterRegistry
    {
        ParameterRegistry();

        virtual ~ParameterRegistry() = default;

        const std::map<std::string, const Parameter>& GetParameters() const;

        const Parameter& Lookup(const std::string& name) const;

        template<class T>
        void AddParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location = std::source_location::current());

        void AssertParameterIsUnique(const std::string& name) const;

    private:
        void AddParameter(Parameter parameter);

        template<class T>
        void AddBuiltinParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location = std::source_location::current());

        template<class T>
        void AddParameter(Parameter parameter, std::function<T(const cucumber::messages::group&)> converter);

        std::map<std::string, const Parameter> parametersByName;
    };

    template<class T>
    void ParameterRegistry::AddParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location)
    {
        AddParameter(Parameter{ name, regex, false, false, location }, converter);
    }

    template<class T>
    void ParameterRegistry::AddBuiltinParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location)
    {
        AddParameter(Parameter{ name, regex, true, false, location }, converter);
    }

    template<class T>
    void ParameterRegistry::AddParameter(Parameter parameter, std::function<T(const cucumber::messages::group&)> converter)
    {
        AssertParameterIsUnique(parameter.name);

        AddParameter(parameter);

        ConverterTypeMap<T>::Instance().emplace(parameter.name, converter);
    }
}

#endif
