#ifndef FORMATTER_FORMATTER_HPP
#define FORMATTER_FORMATTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>

namespace cucumber_cpp::library::formatter
{
    struct Formatter
        : util::Listener
    {
        Formatter(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const helper::EventDataCollector& eventDataCollector, std::ostream& outputStream = std::cout);
        virtual ~Formatter() = default;

    protected:
        virtual void OnEnvelope(const cucumber::messages::envelope& envelope) = 0;

        support::SupportCodeLibrary& supportCodeLibrary;
        util::Broadcaster& broadcaster;
        const helper::EventDataCollector& eventDataCollector;
        std::ostream& outputStream;
    };
}

#endif
