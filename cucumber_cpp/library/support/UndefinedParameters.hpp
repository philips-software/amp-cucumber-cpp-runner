#ifndef SUPPORT_UNDEFINED_PARAMETERS_HPP
#define SUPPORT_UNDEFINED_PARAMETERS_HPP

#include "cucumber/messages/undefined_parameter_type.hpp"
#include <list>

namespace cucumber_cpp::library::support
{
    struct UndefinedParameters
    {
        std::list<cucumber::messages::undefined_parameter_type> definitions;
    };
}

#endif
