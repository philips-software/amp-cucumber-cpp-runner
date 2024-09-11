#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include "cucumber-cpp/InternalError.hpp"
#include <cstddef>
#include <source_location>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    template<class To>
    inline To StringTo(const std::string& s, std::source_location sourceLocation = std::source_location::current())
    {
        std::istringstream stream{ s };
        To to;
        stream >> to;
        if (stream.fail())
            throw InternalError{ "Cannnot convert parameter \"" + s + "\"", sourceLocation };
        return to;
    }

    template<>
    inline std::string StringTo<std::string>(const std::string& s, std::source_location /*sourceLocation*/)
    {
        return s;
    }

    struct Body
    {
        virtual ~Body() = default;

        virtual void Execute(const std::vector<std::string>& args = {}) = 0;

    protected:
        template<class T, class... Args, std::size_t... I>
        void InvokeWithArgImpl(T* t, const std::vector<std::string>& args, void (T::*ptr)(Args...), std::index_sequence<I...> /*seq*/) const
        {
            (t->*ptr)(StringTo<std::remove_cvref_t<Args>>(args[I])...);
        }

        template<class T, class... Args>
        void InvokeWithArg(T* t, const std::vector<std::string>& args, void (T::*ptr)(Args...)) const
        {
            InvokeWithArgImpl(t, args, ptr, std::make_index_sequence<sizeof...(Args)>{});
        }
    };
}

#endif
