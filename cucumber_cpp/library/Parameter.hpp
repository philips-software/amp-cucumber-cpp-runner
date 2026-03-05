#ifndef LIBRARY_PARAMETER_HPP
#define LIBRARY_PARAMETER_HPP

// IWYU pragma: private, include "cucumber_cpp/Steps.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace cucumber_cpp::library::detail
{
    template<class T>
    T TransformParameterArg([[maybe_unused]] const T& _, const std::optional<std::string>& arg)
    {
        return arg.value();
    }

    template<class T>
    std::optional<T> TransformParameterArg([[maybe_unused]] const std::optional<T>& _, const std::optional<std::string>& arg)
    {
        return arg;
    }
}

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define PARAMETER_STRUCT CONCAT(ParameterImpl, __LINE__)

#define PARAMETER_BODY(Type, Info, Args)                                                                                                                                                                                                      \
    namespace                                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                         \
        struct PARAMETER_STRUCT                                                                                                                                                                                                               \
        {                                                                                                                                                                                                                                     \
            static std::optional<Type> Transform(const cucumber_cpp::library::cucumber_expression::ConvertFunctionArg& group)                                                                                                                 \
            {                                                                                                                                                                                                                                 \
                return TransformFn(group, static_cast<void(*) Args>(nullptr));                                                                                                                                                                \
            }                                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                              \
            template<class... TArgs>                                                                                                                                                                                                          \
            static std::optional<Type> TransformFn(const cucumber_cpp::library::cucumber_expression::ConvertFunctionArg& group, void (* /* unused */)(TArgs...))                                                                              \
            {                                                                                                                                                                                                                                 \
                return TransformFn<TArgs...>(group, std::make_index_sequence<sizeof...(TArgs)>{});                                                                                                                                            \
            }                                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                              \
            template<class... TArgs, std::size_t... I>                                                                                                                                                                                        \
            static std::optional<Type> TransformFn(const cucumber_cpp::library::cucumber_expression::ConvertFunctionArg& group, std::index_sequence<I...> /*unused*/)                                                                         \
            {                                                                                                                                                                                                                                 \
                return TransformFn(cucumber_cpp::library::detail::TransformParameterArg(std::remove_cvref_t<TArgs>{}, group[I])...);                                                                                                          \
            }                                                                                                                                                                                                                                 \
            static std::optional<Type> TransformFn Args;                                                                                                                                                                                      \
            static const std::size_t ID;                                                                                                                                                                                                      \
        };                                                                                                                                                                                                                                    \
    }                                                                                                                                                                                                                                         \
    const std::size_t PARAMETER_STRUCT::ID = cucumber_cpp::library::support::DefinitionRegistration::Instance().Register<PARAMETER_STRUCT, std::optional<Type>>(cucumber_cpp::library::cucumber_expression::CustomParameterEntryParams Info); \
    std::optional<Type> PARAMETER_STRUCT::TransformFn Args

#define PARAMETER(Type, Info, Args) PARAMETER_BODY(Type, Info, Args)

#endif
