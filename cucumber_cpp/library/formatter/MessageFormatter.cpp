#include "cucumber_cpp/library/formatter/MessageFormatter.hpp"
#include "cucumber/messages/Envelope.hpp"

namespace cucumber_cpp::library::formatter
{
    void MessageFormatter::OnEnvelope(const cucumber::messages::Envelope& envelope)
    {
        outputStream << envelope.to_json() << "\n";
    }
}
