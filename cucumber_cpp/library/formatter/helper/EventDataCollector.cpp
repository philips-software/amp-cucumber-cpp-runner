#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <ranges>
#include <string>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    EventDataCollector::EventDataCollector(util::Broadcaster& broadcaster)
        : util::Listener{ broadcaster, [this](const cucumber::messages::envelope& envelope)
            {
                OnEnvelope(envelope);
            } }
    {}

    const cucumber::messages::gherkin_document& EventDataCollector::GetGherkinDocument(std::string uri) const
    {
        return gherkinDocumentMap.at(uri);
    }

    const cucumber::messages::pickle& EventDataCollector::GetPickle(std::string id) const
    {
        return pickleMap.at(id);
    }

    const cucumber::messages::test_case& EventDataCollector::GetTestCase(std::string id) const
    {
        return testCaseMap.at(id);
    }

    std::vector<TestCaseAttempt> EventDataCollector::GetTestCaseAttempts() const
    {
        std::vector<TestCaseAttempt> attempts{};
        for (const auto& key : testCaseAttemptDataMap | std::views::keys)
            attempts.emplace_back(GetTestCaseAttempt(key));
        return attempts;
    }

    void EventDataCollector::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.gherkin_document)
        {
            gherkinDocumentMap.emplace(envelope.gherkin_document->uri.value(), *envelope.gherkin_document);
        }
        else if (envelope.pickle)
        {
            pickleMap.emplace(envelope.pickle->id, *envelope.pickle);
        }
        else if (envelope.undefined_parameter_type)
        {
            undefinedParameterTypes.emplace_back(*envelope.undefined_parameter_type);
        }
        else if (envelope.test_case)
        {
            testCaseMap.emplace(envelope.test_case->id, *envelope.test_case);
        }
        else if (envelope.test_case_started)
        {
            InitTestCaseAttempt(*envelope.test_case_started);
        }
        else if (envelope.attachment)
        {
            StoreAttachment(*envelope.attachment);
        }
        else if (envelope.test_step_finished)
        {
            StoreTestStepResult(*envelope.test_step_finished);
        }
        else if (envelope.test_case_finished)
        {
            StoreTestCaseResult(*envelope.test_case_finished);
        }
    }

    void EventDataCollector::InitTestCaseAttempt(const cucumber::messages::test_case_started& testCaseStarted)
    {
        testCaseAttemptDataMap.emplace(testCaseStarted.id,
            TestCaseAttemptData{
                .attempt = testCaseStarted.attempt,
                .willBeRetried = false,
                .testCaseId = testCaseStarted.test_case_id,
                .worstTestStepResult = {
                    .status = cucumber::messages::test_step_result_status::UNKNOWN,
                },
            });
    }

    void EventDataCollector::StoreAttachment(const cucumber::messages::attachment& attachment)
    {
        auto& testCaseAttemptData = testCaseAttemptDataMap.at(*attachment.test_case_started_id);
        testCaseAttemptData.stepAttachments[*attachment.test_step_id].emplace_back(attachment);
    }

    void EventDataCollector::StoreTestStepResult(const cucumber::messages::test_step_finished& testStepFinished)
    {
        auto& testCaseAttemptData = testCaseAttemptDataMap.at(testStepFinished.test_case_started_id);
        testCaseAttemptData.stepResults[testStepFinished.test_step_id] = testStepFinished.test_step_result;
    }

    void EventDataCollector::StoreTestCaseResult(const cucumber::messages::test_case_finished& testCaseFinished)
    {
        auto& testCaseAttemptData = testCaseAttemptDataMap.at(testCaseFinished.test_case_started_id);

        const auto allStepResults = testCaseAttemptData.stepResults | std::views::values;
        std::vector<cucumber::messages::test_step_result> stepResults{ allStepResults.begin(), allStepResults.end() };

        testCaseAttemptData.worstTestStepResult = util::GetWorstTestStepResult(stepResults);
        testCaseAttemptData.willBeRetried = testCaseFinished.will_be_retried;
    }

    TestCaseAttempt EventDataCollector::GetTestCaseAttempt(const std::string& testCaseStartedId) const
    {
        const auto& testCaseAttemptData = testCaseAttemptDataMap.at(testCaseStartedId);
        const auto& testCase = testCaseMap.at(testCaseAttemptData.testCaseId);
        const auto& pickle = pickleMap.at(testCase.pickle_id);
        return {
            .attempt = testCaseAttemptData.attempt,
            .willBeRetried = testCaseAttemptData.willBeRetried,
            .gherkinDocument = gherkinDocumentMap.at(pickle.uri),
            .pickle = pickle,
            .stepAttachments = testCaseAttemptData.stepAttachments,
            .stepResults = testCaseAttemptData.stepResults,
            .testCase = testCase,
            .worstTestStepResult = testCaseAttemptData.worstTestStepResult,
        };
    }
}
