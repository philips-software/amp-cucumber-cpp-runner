#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/StringTo.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#undef GTEST_MESSAGE_AT_
#define GTEST_MESSAGE_AT_(file, line, message, result_type)                                 \
    cucumber_cpp::library::engine::CucumberAssertHelper(result_type, file, line, message) = \
        ::testing::Message()

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                                                                                       \
    namespace                                                                                                                                \
    {                                                                                                                                        \
        struct BODY_STRUCT : cucumber_cpp::library::Body                                                                                     \
            , base                                                                                                                           \
        {                                                                                                                                    \
            /* Workaround namespaces in `base`. For example `base` = Foo::Bar. */                                                            \
            /* Then the result would be Foo::Bar::Foo::Bar which is invalid */                                                               \
            using myBase = base;                                                                                                             \
            using myBase::myBase;                                                                                                            \
                                                                                                                                             \
            void Execute(const std::variant<std::vector<std::string>, std::vector<std::any>>& args) override                                 \
            {                                                                                                                                \
                cucumber_cpp::library::SetUpTearDownWrapper wrapper{ *this };                                                                \
                EXPECT_NO_THROW(ExecuteWithArgs(args, static_cast<void(*) targs>(nullptr)));                                                 \
            }                                                                                                                                \
                                                                                                                                             \
            template<class... TArgs>                                                                                                         \
            void ExecuteWithArgs(const std::variant<std::vector<std::string>, std::vector<std::any>>& args, void (* /* unused */)(TArgs...)) \
            {                                                                                                                                \
                if (std::holds_alternative<std::vector<std::string>>(args))                                                                  \
                    ExecuteWithArgs<TArgs...>(std::get<std::vector<std::string>>(args), std::make_index_sequence<sizeof...(TArgs)>{});       \
                else                                                                                                                         \
                    ExecuteWithArgs<TArgs...>(std::get<std::vector<std::any>>(args), std::make_index_sequence<sizeof...(TArgs)>{});          \
            }                                                                                                                                \
                                                                                                                                             \
            template<class... TArgs, std::size_t... I>                                                                                       \
            void ExecuteWithArgs(const std::vector<std::string>& args, std::index_sequence<I...> /*unused*/)                                 \
            {                                                                                                                                \
                ExecuteWithArgs(cucumber_cpp::library::engine::StringTo<std::remove_cvref_t<TArgs>>(args[I])...);                            \
            }                                                                                                                                \
                                                                                                                                             \
            template<class... TArgs, std::size_t... I>                                                                                       \
            void ExecuteWithArgs(const std::vector<std::any>& args, std::index_sequence<I...> /*unused*/)                                    \
            {                                                                                                                                \
                ExecuteWithArgs(std::any_cast<std::remove_cvref_t<TArgs>>(args[I])...);                                                      \
            }                                                                                                                                \
                                                                                                                                             \
        private:                                                                                                                             \
            void ExecuteWithArgs targs;                                                                                                      \
            static const std::size_t ID;                                                                                                     \
        };                                                                                                                                   \
    }                                                                                                                                        \
    const std::size_t BODY_STRUCT::ID = registration<BODY_STRUCT>(matcher, type);                                                            \
    void BODY_STRUCT::ExecuteWithArgs targs

#endif
