#ifndef SUPPORT_UNDEFINED_PARAMETERS_HPP
#define SUPPORT_UNDEFINED_PARAMETERS_HPP

#include "cucumber/messages/UndefinedParameterType.hpp"
#include <list>

namespace cucumber_cpp::library::support
{
    struct UndefinedParameters
    {
        std::list<cucumber::messages::UndefinedParameterType> definitions;
    };
}

#endif
