
#include "cucumber_cpp/library/formatter/JunitXmlFormatter.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pugixml.hpp"
#include <cstddef>
#include <list>
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        enum class FailureKind
        {
            failure,
            skipped
        };

        struct ReportFailure
        {
            FailureKind kind;
            std::optional<std::string> type;
            std::optional<std::string> message;
            std::optional<std::string> stack;
        };

        struct ReportTestCase
        {
            std::string classname;
            std::string name;
            std::size_t time;
            std::optional<ReportFailure> failure;
            std::string output;
        };

        struct ReportSuite
        {
            cucumber::messages::duration time;
            std::size_t tests;
            std::size_t skipped;
            std::size_t failures;
            std::size_t errors;
            std::list<ReportTestCase> testCases;
            std::optional<std::string> timestamp;
        };

        std::optional<ReportFailure> MakeFailure(query::Query& query, const cucumber::messages::test_case_started& testCaseStarted)
        {
            return std::nullopt;
        }

        std::string MakeOutput(query::Query& query, const cucumber::messages::test_case_started& testCaseStarted)
        {
            return "";
        }

        std::list<ReportTestCase> MakeTestCases(query::Query& query, const std::optional<std::string>& testClassName)
        {
            std::list<ReportTestCase> testCases;

            const auto allTestCaseStarted = query.FindAllTestCaseStarted();
            for (const auto testCaseStartedPtr : allTestCaseStarted)
            {
                const auto& pickle = query.FindPickleBy(*testCaseStartedPtr);
                const auto& lineage = query.FindLineageByPickle(pickle);
                const auto& testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(*testCaseStartedPtr);

                testCases.emplace_back(testClassName.has_value()
                                           ? testClassName.value()
                                       : lineage.feature
                                           ? lineage.feature->name
                                           : pickle.uri,
                    query::NamingStrategy::Reduce(lineage, pickle), query.FindTestCaseDurationBy(*testCaseStartedPtr).seconds, MakeFailure(query, *testCaseStartedPtr), MakeOutput(query, *testCaseStartedPtr));
            }

            return testCases;
        }

        ReportSuite MakeReport(query::Query& query, const std::optional<std::string>& testClassName)
        {
            const auto& statuses = query.CountMostSevereTestStepResultStatus();

            return {
                .time = query.FindTestRunDuration(),
                .tests = query.CountTestCasesStarted(),
                .skipped = statuses.at(cucumber::messages::test_step_result_status::SKIPPED),
                .failures = statuses.at(cucumber::messages::test_step_result_status::UNKNOWN) +
                            statuses.at(cucumber::messages::test_step_result_status::PENDING) +
                            statuses.at(cucumber::messages::test_step_result_status::UNDEFINED) +
                            statuses.at(cucumber::messages::test_step_result_status::AMBIGUOUS) +
                            statuses.at(cucumber::messages::test_step_result_status::FAILED),
                .errors = 0,
                .testCases = MakeTestCases(query, testClassName),
            };
        }
    }

    JunitXmlFormatter::Options::Options(const nlohmann::json& formatOptions)
        : suiteName{ formatOptions.contains("junit") ? formatOptions.at("junit").value("suite_name", "Cucumber") : "Cucumber" }
        , testClassName{ formatOptions.contains("junit") ? formatOptions.at("junit").contains("test_class_name") ? std::make_optional(formatOptions.at("junit")["test_class_name"].get<std::string>()) : std::nullopt : std::nullopt }
    {
    }

    void JunitXmlFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished)
        {
            pugi::xml_document doc;
            const auto& report = MakeReport(query, options.testClassName);

            auto testSuite = doc.append_child("testsuite");
            testSuite.append_attribute("name").set_value(options.suiteName.c_str());
            testSuite.append_attribute("tests").set_value(static_cast<unsigned int>(report.tests));
            testSuite.append_attribute("skipped").set_value(static_cast<unsigned int>(report.skipped));
            testSuite.append_attribute("failures").set_value(static_cast<unsigned int>(report.failures));
            testSuite.append_attribute("errors").set_value(static_cast<unsigned int>(report.errors));

            if (report.timestamp)
                testSuite.append_attribute("timestamp").set_value(report.timestamp->c_str());

            for (const auto& testCase : report.testCases)
            {
                auto testCaseNode = testSuite.append_child("testcase");
                testCaseNode.append_attribute("classname").set_value(testCase.classname.c_str());
                testCaseNode.append_attribute("name").set_value(testCase.name.c_str());
                testCaseNode.append_attribute("time").set_value(testCase.time);

                if (testCase.failure)
                {
                    auto failureNode = testCaseNode.append_child(testCase.failure->kind == FailureKind::failure ? "failure" : "skipped");

                    if (testCase.failure->kind == FailureKind::failure && testCase.failure->type)
                        failureNode.append_attribute("type").set_value(testCase.failure->type->c_str());

                    if (testCase.failure->message)
                        failureNode.append_attribute("message").set_value(testCase.failure->message->c_str());

                    if (testCase.failure->stack)
                        failureNode.append_child(pugi::node_pcdata).set_value(testCase.failure->stack->c_str());
                }

                if (!testCase.output.empty())
                {
                    auto systemOutNode = testCaseNode.append_child("system-out");
                    systemOutNode.append_child(pugi::node_pcdata).set_value(testCase.output.c_str());
                }
            }

            doc.save(outputStream);
        }
    }
}
