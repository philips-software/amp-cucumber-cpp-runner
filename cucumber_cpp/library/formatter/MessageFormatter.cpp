#include "cucumber_cpp/library/formatter/MessageFormatter.hpp"
#include "cucumber/messages/envelope.hpp"

namespace cucumber_cpp::library::formatter
{
    void MessageFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        outputStream << envelope.to_json() << "\n";
    }
}
