#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/Rtrim.hpp"
#include "nlohmann/json.hpp"
#include "pugixml.hpp"
#include <iostream>
#include <numeric>
#include <ranges>
#include <sstream>

namespace cucumber_cpp::report
{
    namespace
    {
        auto ViewAllCapturedStdStream(const nlohmann::json& json, std::string what)
        {
            return json["steps"] | std::views::filter([what](const nlohmann::json& step)
                                       {
                                           return step.contains(what);
                                       }) |
                   std::views::transform([what](const nlohmann::json& step)
                       {
                           return step[what].get<std::string>();
                       });
        }

        void AppendStdStreamOutput(auto& parent, std::string element, auto view)
        {
            if (!view.empty())
            {

                auto elem = parent.append_child(element.c_str());
                auto str = std::accumulate(std::next(view.begin()), view.end(), view.front());
                Rtrim(str);
                elem.text() = str.c_str();
            }
        }

        std::string AllErrors(const nlohmann::json& json)
        {
            auto stepsWithErrors = json["steps"] | std::views::filter([](const nlohmann::json& step)
                                                       {
                                                           return step.contains("errors");
                                                       });
            auto allMessages = stepsWithErrors | std::views::transform([](const nlohmann::json& step)
                                                     {
                                                         auto messages = step["errors"] | std::views::transform([](const nlohmann::json& json)
                                                                                              {
                                                                                                  return json["message"].get<std::string>();
                                                                                              });
                                                         return std::accumulate(messages.begin(), messages.end(), std::string{});
                                                     });

            return std::accumulate(allMessages.begin(), allMessages.end(), std::string{});
        }
    }

    void JunitReport::GenerateReport(const nlohmann::json& json)
    {
        struct Statistics
        {
            std::uint32_t tests = 0;
            std::uint32_t failures = 0;
            std::uint32_t errors = 0;
            std::uint32_t skipped = 0;

            Statistics& operator+=(Statistics& rhs)
            {
                tests += rhs.tests;
                failures += rhs.failures;
                errors += rhs.errors;
                skipped += rhs.skipped;

                return *this;
            }
        };

        pugi::xml_document doc;

        auto testsuites = doc.append_child("testsuites");

        Statistics allStats;

        testsuites.append_attribute("name").set_value("Test run");
        testsuites.append_attribute("time").set_value(json.value("elapsed", 0.0));

        for (const auto& feature : json["features"])
        {
            auto testsuite = testsuites.append_child("testsuite");
            Statistics featureStatistics;

            auto featureName = feature["ast"]["feature"]["name"].get<std::string>();
            testsuite.append_attribute("name").set_value(featureName.c_str());
            testsuite.append_attribute("time").set_value(feature.value("elapsed", 0.0));

            for (const auto& scenario : feature["scenarios"])
            {
                if (!scenario.contains("result"))
                {
                    continue;
                }

                auto testcase = testsuite.append_child("testcase");

                auto scenarioName = scenario["name"].get<std::string>();
                testcase.append_attribute("name").set_value(scenarioName.c_str());
                testcase.append_attribute("time").set_value(scenario.value("elapsed", 0.0));

                ++featureStatistics.tests;

                if (const auto result = scenario["result"]; result == "success")
                {
                }
                else if (result == "failed")
                {
                    ++featureStatistics.failures;

                    auto failure = testcase.append_child("failure");
                    failure.append_attribute("message").set_value(AllErrors(scenario).c_str());
                }
                else if (result == "error")
                {
                    ++featureStatistics.errors;

                    auto error = testcase.append_child("error");
                    error.append_attribute("message").set_value(AllErrors(scenario).c_str());
                }
                else
                {
                    ++featureStatistics.skipped;

                    auto skipped = testcase.append_child("skipped");

                    if (result == "skipped")
                    {
                        skipped.append_attribute("message").set_value("Test is skipped due to previous errors.");
                    }
                    else if (result == "undefined")
                    {
                        skipped.append_attribute("message").set_value("Test is undefined.");
                    }
                    else if (result == "pending")
                    {
                        skipped.append_attribute("message").set_value("Test is pending.");
                    }
                    else
                    {
                        skipped.append_attribute("message").set_value("Test result unkown.");
                    }
                }

                auto stepsWithStdout = ViewAllCapturedStdStream(scenario, "stdout");
                AppendStdStreamOutput(testcase, "system-out", stepsWithStdout);

                auto stepsWithStderr = ViewAllCapturedStdStream(scenario, "stderr");
                AppendStdStreamOutput(testcase, "system-err", stepsWithStderr);
            }

            if (featureStatistics.tests == 0)
            {
                testsuites.remove_child(testsuite);
            }
            else
            {
                testsuite.append_attribute("tests").set_value(featureStatistics.tests);
                testsuite.append_attribute("failures").set_value(featureStatistics.failures);
                testsuite.append_attribute("errors").set_value(featureStatistics.errors);
                testsuite.append_attribute("skipped").set_value(featureStatistics.skipped);

                allStats += featureStatistics;
            }
        }

        testsuites.append_attribute("tests").set_value(allStats.tests);
        testsuites.append_attribute("failures").set_value(allStats.failures);
        testsuites.append_attribute("errors").set_value(allStats.errors);
        testsuites.append_attribute("skipped").set_value(allStats.skipped);

        doc.save_file("out.xml");
    }
}
