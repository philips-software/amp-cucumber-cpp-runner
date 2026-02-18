#ifndef CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP
#define CUCUMBER_EXPRESSION_PARAMETERREGISTRY_HPP

#include "cucumber/messages/group.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <map>
#include <set>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    using namespace std::literals;

    struct CustomParameterEntryParams
    {
        std::string name;
        std::string regex;
        bool useForSnippets;
    };

    struct CustomParameterEntry
    {
        CustomParameterEntryParams params;

        std::size_t localId{};

        std::source_location location;

        std::strong_ordering operator<=>(const CustomParameterEntry& other) const;
    };

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
            throw ConversionError{ fmt::format("Cannot convert parameter {} in to {}", s, typeid(To).name()) };

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

    struct Parameter
    {
        std::string name;
        std::vector<std::string> regex;
        bool isBuiltin{ false };
        bool useForSnippets{ false };
        std::source_location location;
    };

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

    struct ParameterRegistry
    {
        explicit ParameterRegistry(const std::set<CustomParameterEntry, std::less<>>& customParameters);

        virtual ~ParameterRegistry() = default;

        const std::map<std::string, const Parameter, std::less<>>& GetParameters() const;

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

        std::map<std::string, const Parameter, std::less<>> parametersByName;
    };

    template<class T>
    void ParameterRegistry::AddParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location)
    {
        AddParameter(Parameter{ .name = std::move(name), .regex = std::move(regex), .isBuiltin = false, .useForSnippets = false, .location = location }, converter);
    }

    template<class T>
    void ParameterRegistry::AddBuiltinParameter(std::string name, std::vector<std::string> regex, std::function<T(const cucumber::messages::group&)> converter, std::source_location location)
    {
        AddParameter(Parameter{ .name = std::move(name), .regex = std::move(regex), .isBuiltin = true, .useForSnippets = false, .location = location }, converter);
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
