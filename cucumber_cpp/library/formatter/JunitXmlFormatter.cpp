
#include "cucumber_cpp/library/formatter/JunitXmlFormatter.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/Step.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/query/NamingStrategy.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber_cpp/library/util/Trim.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "nlohmann/json.hpp"
#include "pugixml.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

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

        std::optional<ReportFailure> MakeFailure(cucumber::query::Query& query, const std::shared_ptr<const cucumber::messages::TestCaseStarted>& testCaseStarted)
        {
            const auto result = query.FindMostSevereTestStepResultBy(testCaseStarted);
            if (!result.has_value() || result.value()->status == cucumber::messages::TestStepResultStatus::PASSED)
                return std::nullopt;

            const auto& testStepResultStatus = result.value();

            return std::optional<ReportFailure>{
                std::in_place,
                testStepResultStatus->status == cucumber::messages::TestStepResultStatus::SKIPPED ? FailureKind::skipped : FailureKind::failure,
                testStepResultStatus->exception ? std::make_optional((*testStepResultStatus->exception)->type) : std::nullopt,
                testStepResultStatus->exception ? (*testStepResultStatus->exception)->message : std::nullopt,
                testStepResultStatus->exception ? (*testStepResultStatus->exception)->stackTrace : testStepResultStatus->message,
            };
        }

        std::string FormatStep(const cucumber::messages::Step& gherkinStep, const cucumber::messages::PickleStep& pickleStep, cucumber::messages::TestStepResultStatus status)
        {
            auto statusString = std::string{ cucumber::messages::to_string(status) };
            std::transform(statusString.begin(), statusString.end(), statusString.begin(), [](unsigned char c)
                {
                    return std::tolower(c);
                });

            return fmt::format("{:.<76}{}", util::Trim(gherkinStep.keyword) + " " + util::Trim(pickleStep.text), statusString);
        }

        std::string MakeOutput(cucumber::query::Query& query, const std::shared_ptr<const cucumber::messages::TestCaseStarted>& testCaseStarted)
        {
            const auto testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);
            auto outputView = testStepFinishedAndTestStep |
                              std::views::filter([](const cucumber::query::TestStepFinishedAndTestStep& pair)
                                  {
                                      return pair.testStep->pickleStepId.has_value();
                                  }) |
                              std::views::transform([&query](const cucumber::query::TestStepFinishedAndTestStep& pair)
                                  {
                                      const auto pickleStep = query.FindPickleStepBy(pair.testStep).value();
                                      const auto gherkinStep = query.FindStepBy(pickleStep).value();
                                      return FormatStep(*gherkinStep, *pickleStep, pair.testStepFinished->testStepResult->status);
                                  });

            return fmt::format("\n{}\n", fmt::join(outputView, "\n"));
        }

        std::list<ReportTestCase> MakeTestCases(cucumber::query::Query& query, const std::optional<std::string>& testClassName)
        {
            std::list<ReportTestCase> testCases;

            const auto namingStrategy = cucumber::query::CreateNamingStrategy(cucumber::query::NamingStrategyLength::longName);

            const auto allTestCaseStarted = query.FindAllTestCaseStarted();
            for (const auto& testCaseStartedPtr : allTestCaseStarted)
            {
                const auto pickle = query.FindPickleBy(testCaseStartedPtr).value();
                const auto lineageAndPickle = query.FindLineageBy(testCaseStartedPtr).value();
                const auto& lineage = *lineageAndPickle.lineage;
                const auto durationOpt = query.FindTestCaseDurationBy(testCaseStartedPtr);

                testCases.emplace_back(testClassName.has_value()
                                           ? testClassName.value()
                                       : lineage.feature
                                           ? lineage.feature->name
                                           : pickle->uri,
                    namingStrategy->Reduce(lineage, *pickle),
                    util::DurationToMilliseconds(durationOpt.has_value() ? *durationOpt.value() : cucumber::messages::Duration{}).count(),
                    MakeFailure(query, testCaseStartedPtr),
                    MakeOutput(query, testCaseStartedPtr));
            }

            return testCases;
        }

        ReportSuite MakeReport(cucumber::query::Query& query, const std::optional<std::string>& testClassName)
        {
            const auto statuses = query.CountMostSevereTestStepResultStatus();
            const auto count = [&statuses](cucumber::messages::TestStepResultStatus status)
            {
                const auto it = statuses.find(status);
                return it != statuses.end() ? it->second : std::size_t{ 0 };
            };

            const auto testRunDuration = query.FindTestRunDuration();
            const auto testRunStarted = query.FindTestRunStarted();

            return {
                .time = util::DurationToMilliseconds(testRunDuration.has_value() ? *testRunDuration.value() : cucumber::messages::Duration{}).count(),
                .tests = query.CountTestCasesStarted(),
                .skipped = count(cucumber::messages::TestStepResultStatus::SKIPPED),
                .failures = count(cucumber::messages::TestStepResultStatus::UNKNOWN) +
                            count(cucumber::messages::TestStepResultStatus::PENDING) +
                            count(cucumber::messages::TestStepResultStatus::UNDEFINED) +
                            count(cucumber::messages::TestStepResultStatus::AMBIGUOUS) +
                            count(cucumber::messages::TestStepResultStatus::FAILED),
                .errors = 0,
                .testCases = MakeTestCases(query, testClassName),
                .timestamp = testRunStarted.has_value() ? std::make_optional(util::MakeIso8601Timestamp(*testRunStarted.value()->timestamp)) : std::nullopt,
            };
        }

        void AddTestSuiteAttributes(const ReportSuite& report, const std::string& suiteName, pugi::xml_node& testSuiteNode)
        {
            testSuiteNode.append_attribute("name").set_value(suiteName.c_str());
            testSuiteNode.append_attribute("time").set_value(report.time / 1000.0f);
            testSuiteNode.append_attribute("tests").set_value(static_cast<unsigned int>(report.tests));
            testSuiteNode.append_attribute("skipped").set_value(static_cast<unsigned int>(report.skipped));
            testSuiteNode.append_attribute("failures").set_value(static_cast<unsigned int>(report.failures));
            testSuiteNode.append_attribute("errors").set_value(static_cast<unsigned int>(report.errors));
        }

        void AddTimestamp(const ReportSuite& report, pugi::xml_node& testSuiteNode)
        {
            if (report.timestamp)
                testSuiteNode.append_attribute("timestamp").set_value(report.timestamp->c_str());
        }

        void AddFailure(const ReportTestCase& testCase, pugi::xml_node& testCaseNode)
        {
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
        }

        void AddSystemOut(const ReportTestCase& testCase, pugi::xml_node& testCaseNode)
        {
            if (!testCase.output.empty())
                testCaseNode.append_child("system-out")
                    .append_child(pugi::node_cdata)
                    .set_value(testCase.output.c_str());
        }

        void AddTestCases(const ReportSuite& report, pugi::xml_node& testSuiteNode)
        {
            for (const auto& testCase : report.testCases)
            {
                auto testCaseNode = testSuiteNode.append_child("testcase");
                testCaseNode.append_attribute("classname").set_value(testCase.classname.c_str());
                testCaseNode.append_attribute("name").set_value(testCase.name.c_str());
                testCaseNode.append_attribute("time").set_value(testCase.time / 1000.0f);

                AddFailure(testCase, testCaseNode);
                AddSystemOut(testCase, testCaseNode);
            }
        }
    }

    JunitXmlFormatter::Options::Options(const nlohmann::json& formatOptions)
        : suiteName{ formatOptions.value("suite_name", "Cucumber") }
        , testClassName{ formatOptions.contains("test_class_name") ? std::make_optional(formatOptions.at("test_class_name").get<std::string>()) : std::nullopt }
    {
    }

    void JunitXmlFormatter::OnEnvelope(const cucumber::messages::Envelope& envelope)
    {
        if (envelope.testRunFinished)
            HandleTestRunFinished();
    }

    void JunitXmlFormatter::HandleTestRunFinished()
    {
        const auto& report = MakeReport(query, options.testClassName);

        AddTestSuiteAttributes(report, options.suiteName, testSuiteNode);

        AddTimestamp(report, testSuiteNode);

        AddTestCases(report, testSuiteNode);

        doc.save(outputStream);
    }
}
