#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber/messages/Envelope.hpp"
#include <functional>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::util
{
    Listener::Listener(Broadcaster& broadcaster, const std::function<void(const cucumber::messages::Envelope& envelope)>& onEvent)
        : broadcaster{ broadcaster }
        , onEvent{ onEvent }
    {
        broadcaster.AddListener(this);
    }

    Listener::~Listener()
    {
        broadcaster.RemoveListener(this);
    }

    void Listener::Invoke(const cucumber::messages::Envelope& envelope) const
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

    void Broadcaster::Dispatch(cucumber::messages::Envelope&& envelope)
    {
        const auto& stored = archive.Store(std::move(envelope));
        for (const auto& listener : listeners)
            listener->Invoke(stored);
    }
}
