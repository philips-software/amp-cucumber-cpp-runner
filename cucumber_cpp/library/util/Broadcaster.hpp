#ifndef LIBRARY_20EVENT_EMITTER_HPP
#define LIBRARY_20EVENT_EMITTER_HPP

#include "cucumber/messages/envelope.hpp"
#include <functional>
#include <vector>

namespace cucumber_cpp::library::util
{
    struct Broadcaster;

    struct Listener
    {
        explicit Listener(Broadcaster& broadcaster, const std::function<void(const cucumber::messages::envelope& envelope)>& onEvent);

        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) = delete;
        Listener& operator=(Listener&&) = delete;

        ~Listener();

        void Invoke(const cucumber::messages::envelope& envelope) const;

    private:
        Broadcaster& broadcaster;
        std::function<void(const cucumber::messages::envelope& envelope)> onEvent;
    };

    struct Broadcaster
    {
        void AddListener(Listener* listener);
        void RemoveListener(Listener* listener);

        void BroadcastEvent(const cucumber::messages::envelope& envelope);

    private:
        std::vector<Listener*> listeners;
    };
}

#endif
