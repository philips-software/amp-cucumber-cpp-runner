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
#include <functional>
#include <memory>
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

        void BroadcastEvent(std::shared_ptr<cucumber::messages::Attachment> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::ExternalAttachment> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::GherkinDocument> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::Hook> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::Meta> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::ParameterType> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::ParseError> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::Pickle> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::Suggestion> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::Source> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::StepDefinition> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestCase> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestCaseFinished> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestCaseStarted> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunFinished> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunStarted> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestStepFinished> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestStepStarted> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunHookStarted> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunHookFinished> message) const;
        void BroadcastEvent(std::shared_ptr<cucumber::messages::UndefinedParameterType> message) const;

    private:
        void BroadcastEvent(const cucumber::messages::Envelope& envelope) const;

        std::vector<Listener*> listeners;
    };
}

#endif
