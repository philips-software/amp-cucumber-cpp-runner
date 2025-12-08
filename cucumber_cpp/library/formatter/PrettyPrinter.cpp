#include "cucumber_cpp/library/formatter/PrettyPrinter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include "cucumber_cpp/library/formatter/helper/PickleParser.hpp"
#include <algorithm>

namespace cucumber_cpp::library::formatter
{
    void PrettyPrinter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_case_started)
        {
            CalculateIndent(envelope.test_case_started.value());
        }
    }

    void PrettyPrinter::CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted)
    {
        const auto& testCase = eventDataCollector.GetTestCase(testCaseStarted.test_case_id);
        const auto& pickle = eventDataCollector.GetPickle(testCase.pickle_id);
        const auto scenarioMap = helper::GetGherkinScenarioMap(eventDataCollector.GetGherkinDocument(pickle.uri));
        const auto pickleStepMap = helper::GetPickleStepMap(pickle);
        const auto& scenario = scenarioMap.at(pickle.ast_node_ids[0]);

        const auto maxScenarioLength = scenario.name.length();
        auto maxStepLength = std::size_t{ 0 };
        for (const auto& testStep : testCase.test_steps)
        {
            if (testStep.pickle_step_id)
            {
                const auto& pickleStep = pickleStepMap.at(testStep.pickle_step_id.value());
                if (pickleStep.text.length() > maxStepLength)
                {
                    maxStepLength = pickleStep.text.length();
                }
            }
        }

        scenarioIndent = std::max(maxScenarioLength, maxStepLength) + 2;
    }
}
