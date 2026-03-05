#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "nlohmann/json_fwd.hpp"
#include <cstdio>
#include <ostream>

namespace cucumber_cpp::library::formatter
{
    Formatter::Formatter(support::SupportCodeLibrary& supportCodeLibrary, query::Query& query, const nlohmann::json& formatOptions, std::ostream& outputStream)
        : util::Listener{ query, [this](const cucumber::messages::envelope& envelope)
            {
                OnEnvelope(envelope);
            } }
        , supportCodeLibrary{ supportCodeLibrary }
        , query{ query }
        , formatOptions{ formatOptions }
        , outputStream{ outputStream }
    {
    }
}
