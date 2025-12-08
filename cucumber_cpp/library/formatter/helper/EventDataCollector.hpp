#ifndef HELPER_EVENT_DATA_COLLECTOR_HPP
#define HELPER_EVENT_DATA_COLLECTOR_HPP

#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/undefined_parameter_type.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    struct TestCaseAttemptData
    {
        std::size_t attempt;
        bool willBeRetried;
        std::string testCaseId;
        std::map<std::string, std::vector<cucumber::messages::attachment>> stepAttachments;
        std::map<std::string, cucumber::messages::test_step_result> stepResults;
        cucumber::messages::test_step_result worstTestStepResult;
    };

    struct TestCaseAttempt
    {
        std::size_t attempt;
        bool willBeRetried;
        const cucumber::messages::gherkin_document& gherkinDocument;
        const cucumber::messages::pickle& pickle;
        const std::map<std::string, std::vector<cucumber::messages::attachment>>& stepAttachments;
        const std::map<std::string, cucumber::messages::test_step_result>& stepResults;
        const cucumber::messages::test_case& testCase;
        cucumber::messages::test_step_result worstTestStepResult;
    };

    struct EventDataCollector : util::Listener
    {
        explicit EventDataCollector(util::Broadcaster& broadcaster);

        const cucumber::messages::gherkin_document& GetGherkinDocument(std::string uri) const;
        const cucumber::messages::pickle& GetPickle(std::string id) const;
        const cucumber::messages::test_case& GetTestCase(std::string id) const;

        std::vector<TestCaseAttempt> GetTestCaseAttempts() const;

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope);

        void InitTestCaseAttempt(const cucumber::messages::test_case_started& testCaseStarted);
        void StoreAttachment(const cucumber::messages::attachment& attachment);
        void StoreTestStepResult(const cucumber::messages::test_step_finished& testStepFinished);
        void StoreTestCaseResult(const cucumber::messages::test_case_finished& testCaseFinished);

        TestCaseAttempt GetTestCaseAttempt(const std::string& testCaseStartedId) const;

        std::map<std::string, const cucumber::messages::gherkin_document> gherkinDocumentMap;
        std::map<std::string, const cucumber::messages::pickle> pickleMap;
        std::map<std::string, const cucumber::messages::test_case> testCaseMap;
        std::map<std::string, TestCaseAttemptData> testCaseAttemptDataMap;
        std::vector<cucumber::messages::undefined_parameter_type> undefinedParameterTypes;
    };
}

#endif
