#ifndef LIBRARY_20EVENT_EMITTER_HPP
#define LIBRARY_20EVENT_EMITTER_HPP

#include <functional>
#include <vector>

namespace cucumber::messages
{
    struct envelope;
}

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
        virtual ~Broadcaster() = default;

        virtual void AddListener(Listener* listener) = 0;
        virtual void RemoveListener(Listener* listener) = 0;

        virtual void BroadcastEvent(const cucumber::messages::envelope& envelope) = 0;
    };

    struct BroadcasterImpl : Broadcaster
    {
        void AddListener(Listener* listener) override;
        void RemoveListener(Listener* listener) override;

        void BroadcastEvent(const cucumber::messages::envelope& envelope) override;

    private:
        std::vector<Listener*> listeners;
    };
}

#endif
