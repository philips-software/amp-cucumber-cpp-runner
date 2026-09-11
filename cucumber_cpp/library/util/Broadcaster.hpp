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

        ~Listener();

        void Invoke(const cucumber::messages::Envelope& envelope) const;

    private:
        Broadcaster& broadcaster;
        std::function<void(const cucumber::messages::Envelope& envelope)> onEvent;
    };

    struct Broadcaster
    {
        void AddListener(Listener* listener);
        void RemoveListener(Listener* listener);

        void BroadcastEvent(cucumber::messages::Attachment message) const;
        void BroadcastEvent(cucumber::messages::ExternalAttachment message) const;
        void BroadcastEvent(cucumber::messages::GherkinDocument message) const;
        void BroadcastEvent(cucumber::messages::Hook message) const;
        void BroadcastEvent(cucumber::messages::Meta message) const;
        void BroadcastEvent(cucumber::messages::ParameterType message) const;
        void BroadcastEvent(cucumber::messages::ParseError message) const;
        void BroadcastEvent(cucumber::messages::Pickle message) const;
        void BroadcastEvent(cucumber::messages::Suggestion message) const;
        void BroadcastEvent(cucumber::messages::Source message) const;
        void BroadcastEvent(cucumber::messages::StepDefinition message) const;
        void BroadcastEvent(cucumber::messages::TestCase message) const;
        void BroadcastEvent(cucumber::messages::TestCaseFinished message) const;
        void BroadcastEvent(cucumber::messages::TestCaseStarted message) const;
        void BroadcastEvent(cucumber::messages::TestRunFinished message) const;
        void BroadcastEvent(cucumber::messages::TestRunStarted message) const;
        void BroadcastEvent(cucumber::messages::TestStepFinished message) const;
        void BroadcastEvent(cucumber::messages::TestStepStarted message) const;
        void BroadcastEvent(cucumber::messages::TestRunHookStarted message) const;
        void BroadcastEvent(cucumber::messages::TestRunHookFinished message) const;
        void BroadcastEvent(cucumber::messages::UndefinedParameterType message) const;

    private:
        void BroadcastEvent(cucumber::messages::Envelope&& envelope) const;

        mutable cucumber::query::EnvelopeArchive archive;
        std::vector<Listener*> listeners;
    };
}

#endif
