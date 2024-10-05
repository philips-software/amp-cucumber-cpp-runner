#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber_cpp/library/Body.hpp"
#include <cstddef>
#include <functional>

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                                                         \
    namespace                                                                                                  \
    {                                                                                                          \
        struct BODY_STRUCT : cucumber_cpp::Body                                                                \
            , base                                                                                             \
        {                                                                                                      \
            using myBase = base;                                                                               \
            using myBase::myBase;                                                                              \
            void Execute(const std::vector<std::string>& args) override                                        \
            {                                                                                                  \
                SetUp();                                                                                       \
                try                                                                                            \
                {                                                                                              \
                    InvokeWithArg(args, std::function<void targs>{});                                          \
                }                                                                                              \
                catch (...)                                                                                    \
                {                                                                                              \
                    TearDown();                                                                                \
                    throw;                                                                                     \
                }                                                                                              \
                TearDown();                                                                                    \
            }                                                                                                  \
                                                                                                               \
            template<class... TArgs, std::size_t... I>                                                         \
            void InvokeWithArg(const std::vector<std::string>& args, std::index_sequence<I...> /*unused*/)     \
            {                                                                                                  \
                ExecuteWithArgs(cucumber_cpp::StringTo<std::remove_cvref_t<TArgs>>(args[I])...);               \
            }                                                                                                  \
                                                                                                               \
            template<class... TArgs>                                                                           \
            void InvokeWithArg(const std::vector<std::string>& args, std::function<void(TArgs...)> /*unused*/) \
            {                                                                                                  \
                InvokeWithArg<TArgs...>(args, std::make_index_sequence<sizeof...(TArgs)>{});                   \
            }                                                                                                  \
                                                                                                               \
        private:                                                                                               \
            void ExecuteWithArgs targs;                                                                        \
            static const std::size_t ID;                                                                       \
        };                                                                                                     \
    }                                                                                                          \
    const std::size_t BODY_STRUCT::ID = cucumber_cpp::registration<BODY_STRUCT>(matcher, type);                \
    void BODY_STRUCT::ExecuteWithArgs targs

#endif
