
#include "cucumber_cpp/library/formatter/JunitXmlFormatter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "nlohmann/json_fwd.hpp"
#include "pugixml.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        [[nodiscard]] std::string trim(const std::string& str)
        {
            const auto start = str.find_first_not_of(" \t\n\r");
            if (start == std::string::npos)
                return "";

            const auto end = str.find_last_not_of(" \t\n\r");
            return str.substr(start, end - start + 1);
        }

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
            std::int64_t time;
            std::optional<ReportFailure> failure;
            std::string output;
        };

        struct ReportSuite
        {
            std::int64_t time;
            std::size_t tests;
            std::size_t skipped;
            std::size_t failures;
            std::size_t errors;
            std::list<ReportTestCase> testCases;
            std::optional<std::string> timestamp;
        };

        std::optional<ReportFailure> MakeFailure(query::Query& query, const cucumber::messages::test_case_started& testCaseStarted)
        {
            const auto result = query.FindMostSevereTestStepResultBy(testCaseStarted);
            if (!result.has_value() || result.value()->status == cucumber::messages::test_step_result_status::PASSED)
                return std::nullopt;

            const auto& testStepResultStatus = result.value();

            return std::optional<ReportFailure>{
                std::in_place,
                testStepResultStatus->status == cucumber::messages::test_step_result_status::SKIPPED ? FailureKind::skipped : FailureKind::failure,
                testStepResultStatus->exception ? std::make_optional(testStepResultStatus->exception->type) : std::nullopt,
                testStepResultStatus->exception ? testStepResultStatus->exception->message : std::nullopt,
                testStepResultStatus->exception ? testStepResultStatus->exception->stack_trace : testStepResultStatus->message,
            };
        }

        std::string FormatStep(const cucumber::messages::step& gherkinStep, const cucumber::messages::pickle_step& pickleStep, cucumber::messages::test_step_result_status status)
        {
            auto statusString = std::string{ cucumber::messages::to_string(status) };
            std::transform(statusString.begin(), statusString.end(), statusString.begin(), [](unsigned char c)
                {
                    return std::tolower(c);
                });

            return fmt::format("{:.<76}{}", trim(gherkinStep.keyword) + " " + trim(pickleStep.text), statusString);
        }

        std::string MakeOutput(query::Query& query, const cucumber::messages::test_case_started& testCaseStarted)
        {
            const auto& testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);
            auto outputView = testStepFinishedAndTestStep |
                              std::views::filter([](const std::pair<const cucumber::messages::test_step_finished*, const cucumber::messages::test_step*>& pair)
                                  {
                                      const auto [_, testStep] = pair;
                                      return testStep->pickle_step_id.has_value();
                                  }) |
                              std::views::transform([&query](const std::pair<const cucumber::messages::test_step_finished*, const cucumber::messages::test_step*>& pair)
                                  {
                                      const auto [testStepFinished, testStep] = pair;
                                      const auto& pickleStep = *query.FindPickleStepBy(*testStep);
                                      const auto& gherkinStep = query.FindStepBy(pickleStep);
                                      return FormatStep(gherkinStep, pickleStep, testStepFinished->test_step_result.status);
                                  });

            return fmt::format("\n{}\n", fmt::join(outputView, "\n"));
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
                    query::NamingStrategy::Reduce(lineage, pickle),
                    util::DurationToMilliseconds(query.FindTestCaseDurationBy(*testCaseStartedPtr)).count(),
                    MakeFailure(query, *testCaseStartedPtr),
                    MakeOutput(query, *testCaseStartedPtr));
            }

            return testCases;
        }

        ReportSuite MakeReport(query::Query& query, const std::optional<std::string>& testClassName)
        {
            const auto& statuses = query.CountMostSevereTestStepResultStatus();

            return {
                .time = util::DurationToMilliseconds(query.FindTestRunDuration()).count(),
                .tests = query.CountTestCasesStarted(),
                .skipped = statuses.at(cucumber::messages::test_step_result_status::SKIPPED),
                .failures = statuses.at(cucumber::messages::test_step_result_status::UNKNOWN) +
                            statuses.at(cucumber::messages::test_step_result_status::PENDING) +
                            statuses.at(cucumber::messages::test_step_result_status::UNDEFINED) +
                            statuses.at(cucumber::messages::test_step_result_status::AMBIGUOUS) +
                            statuses.at(cucumber::messages::test_step_result_status::FAILED),
                .errors = 0,
                .testCases = MakeTestCases(query, testClassName),
                .timestamp = util::MakeIso8601Timestamp(query.FindTestRunStarted().timestamp),
            };
        }
    }

    JunitXmlFormatter::Options::Options(const nlohmann::json& formatOptions)
        : suiteName{ formatOptions.value("suite_name", "Cucumber") }
        , testClassName{ formatOptions.contains("test_class_name") ? std::make_optional(formatOptions.at("test_class_name").get<std::string>()) : std::nullopt }
    {
    }

    void JunitXmlFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished)
        {
            const auto& report = MakeReport(query, options.testClassName);

            testSuite.append_attribute("name").set_value(options.suiteName.c_str());
            testSuite.append_attribute("time").set_value(report.time / 1000.0f);
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
                testCaseNode.append_attribute("time").set_value(testCase.time / 1000.0f);

                if (testCase.failure)
                {
                    auto failureNode = testCaseNode.append_child(testCase.failure->kind == FailureKind::failure ? "failure" : "skipped");

                    if (testCase.failure->kind == FailureKind::failure && testCase.failure->type)
                        failureNode.append_attribute("type").set_value(testCase.failure->type->c_str());

                    if (testCase.failure->message)
                        failureNode.append_attribute("message").set_value(testCase.failure->message->c_str());

                    if (testCase.failure->stack)
                        failureNode.append_child(pugi::node_cdata).set_value(testCase.failure->stack->c_str());
                }

                if (!testCase.output.empty())
                {
                    auto systemOutNode = testCaseNode.append_child("system-out");
                    systemOutNode.append_child(pugi::node_cdata).set_value(testCase.output.c_str());
                }
            }

            doc.save(outputStream);
        }
    }
}
