#ifndef CUCUMBER_CPP_INTERNALERROR_HPP
#define CUCUMBER_CPP_INTERNALERROR_HPP

#include <source_location>
#include <stdexcept>
#include <string>

namespace cucumber_cpp::library
{
    struct InternalError : std::runtime_error
    {
        InternalError(const std::string& str, std::source_location sourceLocation = std::source_location::current())
            : std::runtime_error{ str }
            , sourceLocation{ sourceLocation }
        {}

        const std::source_location sourceLocation;
    };
}

#endif
