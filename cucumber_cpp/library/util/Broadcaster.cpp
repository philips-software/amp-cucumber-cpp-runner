#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber/messages/envelope.hpp"
#include <functional>
#include <vector>

namespace cucumber_cpp::library::util
{
    Listener::Listener(Broadcaster& broadcaster, const std::function<void(const cucumber::messages::envelope& envelope)>& onEvent)
        : broadcaster{ broadcaster }
        , onEvent{ onEvent }
    {
        broadcaster.AddListener(this);
    }

    Listener::~Listener()
    {
        broadcaster.RemoveListener(this);
    }

    void Listener::Invoke(const cucumber::messages::envelope& envelope) const
    {
        if (onEvent)
            onEvent(envelope);
    }

    void Broadcaster::AddListener(Listener* listener)
    {
        listeners.push_back(listener);
    }

    void Broadcaster::RemoveListener(Listener* listener)
    {
        std::erase(listeners, listener);
    }

    void Broadcaster::BroadcastEvent(const cucumber::messages::envelope& envelope)
    {
        for (auto& listener : listeners)
            listener->Invoke(envelope);
    }
}
