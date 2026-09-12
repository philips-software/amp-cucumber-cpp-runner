#ifndef LIBRARY_20EVENT_EMITTER_HPP
#define LIBRARY_20EVENT_EMITTER_HPP

#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/ExternalAttachment.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/Meta.hpp"
#include "cucumber/messages/ParameterType.hpp"
#include "cucumber/messages/ParseError.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/Source.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/Suggestion.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber/messages/TestCaseFinished.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestRunHookFinished.hpp"
#include "cucumber/messages/TestRunHookStarted.hpp"
#include "cucumber/messages/TestRunStarted.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepStarted.hpp"
#include "cucumber/messages/UndefinedParameterType.hpp"
#include "cucumber/query/EnvelopeArchive.hpp"
#include <functional>
#include <utility>
#include <vector>

namespace cucumber::messages
{
    struct Envelope;
}

namespace cucumber_cpp::library::util
{
    struct Broadcaster;

    struct Listener
    {
        explicit Listener(Broadcaster& broadcaster, const std::function<void(const cucumber::messages::Envelope& envelope)>& onEvent);

        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) = delete;
        Listener& operator=(Listener&&) = delete;

        virtual ~Listener();

        void Invoke(const cucumber::messages::Envelope& envelope) const;

    private:
        Broadcaster& broadcaster;
        std::function<void(const cucumber::messages::Envelope& envelope)> onEvent;
    };

    struct Broadcaster
    {
        void AddListener(Listener* listener);
        void RemoveListener(Listener* listener);

        template<typename Populate>
        void BroadcastEvent(Populate&& populate)
        {
            cucumber::messages::Envelope envelope;
            std::forward<Populate>(populate)(envelope);
            Dispatch(std::move(envelope));
        }

    private:
        void Dispatch(cucumber::messages::Envelope&& envelope);

        cucumber::query::EnvelopeArchive archive;
        std::vector<Listener*> listeners;
    };
}

#endif
