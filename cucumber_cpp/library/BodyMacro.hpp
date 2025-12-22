#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/StringTo.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                                                                                                                                                                                     \
    namespace                                                                                                                                                                                                                              \
    {                                                                                                                                                                                                                                      \
        struct BODY_STRUCT : cucumber_cpp::library::Body                                                                                                                                                                                   \
            , base                                                                                                                                                                                                                         \
        {                                                                                                                                                                                                                                  \
            /* Workaround namespaces in `base`. For example `base` = Foo::Bar. */                                                                                                                                                          \
            /* Then the result would be Foo::Bar::Foo::Bar which is invalid */                                                                                                                                                             \
            using myBase = base;                                                                                                                                                                                                           \
            using myBase::myBase;                                                                                                                                                                                                          \
                                                                                                                                                                                                                                           \
            void Execute(const cucumber::messages::step_match_arguments_list& args) override                                                                                                                                               \
            {                                                                                                                                                                                                                              \
                cucumber_cpp::library::SetUpTearDownWrapper wrapper{ *this };                                                                                                                                                              \
                /*   ASSERT_NO_THROW(ExecuteWithArgs(args, static_cast<void(*) targs>(nullptr)));   */                                                                                                                                     \
                ExecuteWithArgs(args, static_cast<void(*) targs>(nullptr));                                                                                                                                                                \
            }                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                           \
            template<class... TArgs>                                                                                                                                                                                                       \
            void ExecuteWithArgs(const cucumber::messages::step_match_arguments_list& args, void (* /* unused */)(TArgs...))                                                                                                               \
            {                                                                                                                                                                                                                              \
                ExecuteWithArgs<TArgs...>(args, std::make_index_sequence<sizeof...(TArgs)>{});                                                                                                                                             \
            }                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                           \
            template<class... TArgs, std::size_t... I>                                                                                                                                                                                     \
            void ExecuteWithArgs(const cucumber::messages::step_match_arguments_list& args, std::index_sequence<I...> /*unused*/)                                                                                                          \
            {                                                                                                                                                                                                                              \
                ExecuteWithArgs(cucumber_cpp::library::cucumber_expression::ConverterTypeMap<std::remove_cvref_t<TArgs>>::Instance().at(args.step_match_arguments[I].parameter_type_name.value())(args.step_match_arguments[I].group)...); \
            }                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                           \
        private:                                                                                                                                                                                                                           \
            void ExecuteWithArgs targs;                                                                                                                                                                                                    \
            static const std::size_t ID;                                                                                                                                                                                                   \
        };                                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                                      \
    const std::size_t BODY_STRUCT::ID = registration<BODY_STRUCT>(matcher, type);                                                                                                                                                          \
    void BODY_STRUCT::ExecuteWithArgs targs

#endif
