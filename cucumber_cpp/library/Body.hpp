#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include "cucumber_cpp/library/engine/StringTo.hpp"
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
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
