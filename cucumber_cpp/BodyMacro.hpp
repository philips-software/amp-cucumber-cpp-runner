#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber_cpp/Body.hpp"

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                                          \
    namespace                                                                                   \
    {                                                                                           \
        struct BODY_STRUCT : cucumber_cpp::Body                                                 \
            , base                                                                              \
        {                                                                                       \
            using myBase = base;                                                                \
            using myBase::myBase;                                                               \
            void Execute(const std::vector<std::string>& args) override                         \
            {                                                                                   \
                SetUp();                                                                        \
                try                                                                             \
                {                                                                               \
                    InvokeWithArg(this, args, &BODY_STRUCT::ExecuteWithArgs);                   \
                }                                                                               \
                catch (...)                                                                     \
                {                                                                               \
                    TearDown();                                                                 \
                    throw;                                                                      \
                }                                                                               \
                TearDown();                                                                     \
            }                                                                                   \
                                                                                                \
        private:                                                                                \
            void ExecuteWithArgs targs;                                                         \
            static const std::size_t ID;                                                        \
        };                                                                                      \
    }                                                                                           \
    const std::size_t BODY_STRUCT::ID = cucumber_cpp::registration<BODY_STRUCT>(matcher, type); \
    void BODY_STRUCT::ExecuteWithArgs targs

#endif
