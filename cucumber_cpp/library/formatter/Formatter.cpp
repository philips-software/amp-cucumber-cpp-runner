#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstdio>
#include <ostream>

namespace cucumber_cpp::library::formatter
{
    Formatter::Formatter(support::SupportCodeLibrary& supportCodeLibrary, Query& query, const helper::EventDataCollector& eventDataCollector, std::ostream& outputStream)
        : util::Listener{ query, [this](const cucumber::messages::envelope& envelope)
            {
                OnEnvelope(envelope);
            } }
        , supportCodeLibrary{ supportCodeLibrary }
        , query{ query }
        , eventDataCollector{ eventDataCollector }
        , outputStream{ outputStream }
    {
    }
}
