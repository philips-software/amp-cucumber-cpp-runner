#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "nlohmann/json_fwd.hpp"
#include <cstdio>
#include <ostream>

namespace cucumber_cpp::library::formatter
{
    Formatter::Formatter(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const nlohmann::json& formatOptions, std::ostream& outputStream)
        : util::Listener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
            {
                query.Update(envelope);
                OnEnvelope(envelope);
            } }
        , supportCodeLibrary{ supportCodeLibrary }
        , formatOptions{ formatOptions }
        , outputStream{ outputStream }
    {
    }
}
