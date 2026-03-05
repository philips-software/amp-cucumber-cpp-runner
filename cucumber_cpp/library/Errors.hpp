#ifndef LIBRARY_ERRORS_HPP
#define LIBRARY_ERRORS_HPP

#include <stdexcept>

namespace cucumber_cpp::library
{
    struct InternalError : std::runtime_error
    {
        using runtime_error::runtime_error;
    };
}

#endif
