#ifndef CUCUMBER_CPP_BODYMACRO_HPP
#define CUCUMBER_CPP_BODYMACRO_HPP

#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Body.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <utility>

namespace cucumber_cpp::library::detail
{
    template<class T>
    T TransformArg(const std::string& parameterName, const cucumber_expression::ConvertFunctionArg& parameterArgs)
    {
        return cucumber_expression::TransformArg(T{}, parameterName, parameterArgs);
    }

    template<class Base>
    struct BodyCrtp : support::Body
    {
    private:
        BodyCrtp() = default;
        friend Base;

    public:
        void Execute(const support::ExecuteArgs& args) override
        {
            cucumber_cpp::library::support::SetUpTearDownWrapper wrapper{ *static_cast<Base*>(this) };

            [this, &args]<class... TArgs>(void (Base::* /* unused */)(TArgs...))
            {
                [this, &args]<std::size_t... I>(std::index_sequence<I...> /*unused*/)
                {
                    static_cast<Base*>(this)->ExecuteImpl(TransformArg<std::remove_cvref_t<TArgs>>(args[I].converterName, args[I].converterArgs)...);
                }(std::make_index_sequence<sizeof...(TArgs)>{});
            }(&Base::ExecuteImpl);
        }
    };
}

#define BODY_MATCHER(matcher, ...) matcher
#define BODY_ARGS(matcher, args, ...) args

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define BODY_STRUCT CONCAT(BodyImpl, __LINE__)

#define BODY(matcher, type, targs, registration, base)                            \
    namespace                                                                     \
    {                                                                             \
        struct BODY_STRUCT : cucumber_cpp::library::detail::BodyCrtp<BODY_STRUCT> \
            , base                                                                \
        {                                                                         \
            /* Workaround namespaces in `base`. For example `base` = Foo::Bar. */ \
            /* Then the result would be Foo::Bar::Foo::Bar which is invalid */    \
            using myBase = base;                                                  \
            using myBase::myBase;                                                 \
                                                                                  \
        private:                                                                  \
            friend BodyCrtp;                                                      \
            void ExecuteImpl targs;                                               \
            static const std::size_t ID;                                          \
        };                                                                        \
    }                                                                             \
    const std::size_t BODY_STRUCT::ID = registration<BODY_STRUCT>(matcher, type); \
    void BODY_STRUCT::ExecuteImpl targs

#endif
