#ifndef SUPPORT_TIMESTAMP_HPP
#define SUPPORT_TIMESTAMP_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/timestamp.hpp"
#include <cstddef>

namespace cucumber_cpp::library::support
{
    constexpr std::size_t millisecondsPerSecond = 1e3;
    constexpr std::size_t nanosecondsPerMillisecond = 1e6;
    constexpr std::size_t nanosecondsPerSecond = 1e9;

    cucumber::messages::timestamp TimestampNow();

    cucumber::messages::duration operator-(const cucumber::messages::timestamp& lhs, const cucumber::messages::timestamp& rhs);
}

namespace cucumber::messages
{
    using cucumber_cpp::library::support::operator-;
};

#endif
