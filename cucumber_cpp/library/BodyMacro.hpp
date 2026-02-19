#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber/messages/group.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Body.hpp"
#include <concepts>
#include <cstddef>
#include <gtest/gtest.h>
#include <optional>
#include <ranges>
#include <string>

namespace cucumber_cpp::library::detail
{
    inline std::optional<std::string> ToString(const cucumber::messages::group& group)
    {
        return group.value;
    }

    inline cucumber_expression::ConvertFunctionArg GroupToArgumentGroup(const cucumber::messages::group& group)
    {
        if (group.children.empty())
            return { group.value };

        auto strings = group.children | std::views::transform(ToString);

        return { strings.begin(), strings.end() };
    }

    template<class T>
    T TransformArg(const cucumber::messages::step_match_argument& match)
    {
        return cucumber_expression::TransformArg(T{}, match.parameter_type_name.value_or(""), GroupToArgumentGroup(match.group));
    }
}

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                                                                                     \
    namespace                                                                                                                              \
    {                                                                                                                                      \
        struct BODY_STRUCT : cucumber_cpp::library::support::Body                                                                          \
            , base                                                                                                                         \
        {                                                                                                                                  \
            /* Workaround namespaces in `base`. For example `base` = Foo::Bar. */                                                          \
            /* Then the result would be Foo::Bar::Foo::Bar which is invalid */                                                             \
            using myBase = base;                                                                                                           \
            using myBase::myBase;                                                                                                          \
                                                                                                                                           \
            void Execute(const cucumber::messages::step_match_arguments_list& args) override                                               \
            {                                                                                                                              \
                cucumber_cpp::library::support::SetUpTearDownWrapper wrapper{ *this };                                                     \
                ExecuteWithArgs(args, static_cast<void(*) targs>(nullptr));                                                                \
            }                                                                                                                              \
                                                                                                                                           \
            template<class... TArgs>                                                                                                       \
            void ExecuteWithArgs(const cucumber::messages::step_match_arguments_list& args, void (* /* unused */)(TArgs...))               \
            {                                                                                                                              \
                ExecuteWithArgs<TArgs...>(args, std::make_index_sequence<sizeof...(TArgs)>{});                                             \
            }                                                                                                                              \
                                                                                                                                           \
            template<class... TArgs, std::size_t... I>                                                                                     \
            void ExecuteWithArgs(const cucumber::messages::step_match_arguments_list& args, std::index_sequence<I...> /*unused*/)          \
            {                                                                                                                              \
                ExecuteWithArgs(cucumber_cpp::library::detail::TransformArg<std::remove_cvref_t<TArgs>>(args.step_match_arguments[I])...); \
            }                                                                                                                              \
                                                                                                                                           \
        private:                                                                                                                           \
            void ExecuteWithArgs targs;                                                                                                    \
            static const std::size_t ID;                                                                                                   \
        };                                                                                                                                 \
    }                                                                                                                                      \
    const std::size_t BODY_STRUCT::ID = registration<BODY_STRUCT>(matcher, type);                                                          \
    void BODY_STRUCT::ExecuteWithArgs targs

#endif
