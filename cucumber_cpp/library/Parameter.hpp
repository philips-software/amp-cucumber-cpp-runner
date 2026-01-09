#ifndef LIBRARY_PARAMETER_HPP
#define LIBRARY_PARAMETER_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"

#define PARAMETER_STRUCT CONCAT(ParameterImpl, __LINE__)

#define PARAMETER(Type, ...)                                                                                                                                       \
    namespace                                                                                                                                                      \
    {                                                                                                                                                              \
        struct PARAMETER_STRUCT                                                                                                                                    \
        {                                                                                                                                                          \
            static Type Transform(const cucumber::messages::group& group);                                                                                         \
            static const std::size_t ID;                                                                                                                           \
        };                                                                                                                                                         \
    }                                                                                                                                                              \
    const std::size_t PARAMETER_STRUCT::ID = cucumber_cpp::library::support::DefinitionRegistration::Instance().Register<PARAMETER_STRUCT, Type>({ __VA_ARGS__ }); \
    Type PARAMETER_STRUCT::Transform(const cucumber::messages::group& group)

#endif
