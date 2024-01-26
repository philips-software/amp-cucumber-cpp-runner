#ifndef CUCUMBER_CPP_INTERNALERROR_HPP
#define CUCUMBER_CPP_INTERNALERROR_HPP

#include <source_location>
#include <stdexcept>

namespace cucumber_cpp
{
    struct InternalError : std::runtime_error
    {
        InternalError(std::string str, std::source_location sourceLocation = std::source_location::current())
            : std::runtime_error{ std::move(str) }
            , sourceLocation{ sourceLocation }
        {}

        const std::source_location sourceLocation;
    };
}

#endif
