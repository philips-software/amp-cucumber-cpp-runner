#ifndef SUPPORT_DURATION_HPP
#define SUPPORT_DURATION_HPP

#include "cucumber/messages/duration.hpp"
#include <chrono>
#include <cstddef>

namespace cucumber_cpp::library::support
{
    cucumber::messages::duration MillisecondsToDuration(std::chrono::milliseconds millis);

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::duration& duration);
    cucumber::messages::duration& operator+=(cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs);
}

namespace cucumber::messages
{
    using cucumber_cpp::library::support::operator+=;
};

#endif
