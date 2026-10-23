#ifndef LIBRARY_PARAMETER_MAP_HPP
#define LIBRARY_PARAMETER_MAP_HPP

// IWYU pragma: private, include "cucumber_cpp/Steps.hpp"
// IWYU pragma: friend cucumber_cpp/.*

// IWYU pragma: begin_keep
#include "cucumber_cpp/library/Parameter.hpp"
#include "cucumber_cpp/library/support/ParameterMap.hpp"
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
// IWYU pragma: end_keep

namespace cucumber_cpp::library::detail
{
    template<class T>
    class ParameterMapEntries
    {
    public:
        explicit ParameterMapEntries(std::initializer_list<std::pair<std::string_view, T>> entries)
            : regex{ support::KeysToRegex(Keys(entries)) }
        {
            for (const auto& [key, value] : entries)
                values.emplace(key, value);
        }

        [[nodiscard]] const std::string& Regex() const
        {
            return regex;
        }

        [[nodiscard]] const T& Lookup(const std::string& key) const
        {
            return values.at(key);
        }

    private:
        static std::vector<std::string_view> Keys(std::initializer_list<std::pair<std::string_view, T>> entries)
        {
            std::vector<std::string_view> keys;
            keys.reserve(entries.size());
            for (const auto& entry : entries)
                keys.push_back(entry.first);
            return keys;
        }

        std::string regex;
        std::map<std::string, T, std::less<>> values;
    };
}

#define PARAMETER_MAP_UNPAREN(...) __VA_ARGS__
#define PARAMETER_MAP_NAME(name, snippets) name
#define PARAMETER_MAP_SNIPPETS(name, snippets) snippets
#define PARAMETER_MAP_ENTRIES CONCAT(ParameterMapEntries, __LINE__)

// The entries live in a function-local static so they are initialized on first use,
// which avoids static initialization order issues during parameter registration.
#define PARAMETER_MAP_BODY(Type, Info, Entries)                                                                                        \
    namespace                                                                                                                          \
    {                                                                                                                                  \
        const cucumber_cpp::library::detail::ParameterMapEntries<Type>& PARAMETER_MAP_ENTRIES()                                        \
        {                                                                                                                              \
            static const cucumber_cpp::library::detail::ParameterMapEntries<Type> entries({ PARAMETER_MAP_UNPAREN Entries });          \
            return entries;                                                                                                            \
        }                                                                                                                              \
    }                                                                                                                                  \
    PARAMETER(Type, (PARAMETER_MAP_NAME Info, PARAMETER_MAP_ENTRIES().Regex(), PARAMETER_MAP_SNIPPETS Info), (const std::string& key)) \
    {                                                                                                                                  \
        return PARAMETER_MAP_ENTRIES().Lookup(key);                                                                                    \
    }                                                                                                                                  \
    static_assert(true, "")

#define PARAMETER_MAP(Type, Info, Entries) PARAMETER_MAP_BODY(Type, Info, Entries)

#endif
