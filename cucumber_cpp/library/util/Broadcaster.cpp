#include "cucumber_cpp/library/util/Broadcaster.hpp"
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

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Attachment> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.attachment = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::ExternalAttachment> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.externalAttachment = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::GherkinDocument> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.gherkinDocument = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Hook> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.hook = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Meta> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.meta = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::ParameterType> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.parameterType = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::ParseError> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.parseError = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Pickle> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.pickle = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Suggestion> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.suggestion = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::Source> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.source = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::StepDefinition> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.stepDefinition = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestCase> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testCase = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestCaseFinished> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testCaseFinished = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestCaseStarted> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testCaseStarted = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunFinished> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testRunFinished = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunStarted> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testRunStarted = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestStepFinished> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testStepFinished = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestStepStarted> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testStepStarted = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunHookStarted> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testRunHookStarted = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::TestRunHookFinished> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.testRunHookFinished = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(std::shared_ptr<cucumber::messages::UndefinedParameterType> message) const
    {
        cucumber::messages::Envelope envelope;
        envelope.undefinedParameterType = std::move(message);
        BroadcastEvent(envelope);
    }

    void Broadcaster::BroadcastEvent(const cucumber::messages::Envelope& envelope) const
    {
        for (const auto& listener : listeners)
            listener->Invoke(envelope);
    }
}
