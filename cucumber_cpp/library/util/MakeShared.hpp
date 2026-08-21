#ifndef UTIL_MAKE_SHARED_HPP
#define UTIL_MAKE_SHARED_HPP

#include <memory>
#include <type_traits>
#include <utility>

namespace cucumber_cpp::library::util
{
    template<class T>
    [[nodiscard]] std::shared_ptr<std::decay_t<T>> MakeShared(T&& value)
    {
        return std::make_shared<std::decay_t<T>>(std::forward<T>(value));
    }
}

#endif
