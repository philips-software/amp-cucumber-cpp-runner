#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber-cpp/Body.hpp"

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, args, registration, base)                                           \
    namespace                                                                                   \
    {                                                                                           \
        struct BODY_STRUCT : cucumber_cpp::Body                                                 \
            , base                                                                              \
        {                                                                                       \
            using myBase = base;                                                                \
            using myBase::myBase;                                                               \
            void Execute(const nlohmann::json& parameters = {}) override                        \
            {                                                                                   \
                InvokeWithArg(this, parameters, &BODY_STRUCT::ExecuteWithArgs);                 \
            }                                                                                   \
                                                                                                \
        private:                                                                                \
            void ExecuteWithArgs args;                                                          \
            static const std::size_t ID;                                                        \
        };                                                                                      \
    }                                                                                           \
    const std::size_t BODY_STRUCT::ID = cucumber_cpp::registration<BODY_STRUCT>(matcher, type); \
    void BODY_STRUCT::ExecuteWithArgs args

#endif
