#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "library/support/Duration.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <memory>
#include <ranges>
#include <regex>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#ifndef KIT_FOLDER
#error KIT_FOLDER is not defined
#define KIT_FOLDER ""
#endif

#ifndef KIT_NDJSON_FILE
#error KIT_NDJSON_FILE is not defined
#define KIT_NDJSON_FILE ""
#endif

namespace compatibility
{
    namespace
    {
        struct Devkit
        {
            std::vector<std::filesystem::path> paths;
            std::string tagExpression;
            std::size_t retry;
            std::filesystem::path ndjsonFile;
        };

        Devkit LoadDevkit()
        {
            return {
                .paths = { KIT_FOLDER },
                .tagExpression = "",
                .retry = 0,
                .ndjsonFile = KIT_NDJSON_FILE
            };
        }

        void SanitizeExpectedJson(nlohmann::json& json)
        {
            for (auto& [key, value] : json.items())
            {
                if (value.is_object())
                {
                    SanitizeExpectedJson(value);
                    if (value.size() == 0)
                        json.erase(key);
                }
                else if (value.is_array())
                {
                    auto idx = 0;
                    for (auto& item : value)
                    {
                        if (item.is_object())
                            SanitizeExpectedJson(item);

                        if (item.size() == 0)
                            value.erase(idx);

                        ++idx;
                    }

                    if (value.size() == 0)
                        json.erase(key);
                }
                else if (key == "uri")
                {
                    json[key] = std::regex_replace(value.get<std::string>(), std::regex(R"(samples\/[^\/]+)"), KIT_FOLDER);
                    json[key] = std::regex_replace(value.get<std::string>(), std::regex(R"(\.ts$)"), ".cpp");
                }
                else if (key == "line")
                    json.erase(key);
                else if (key == "start")
                    json.erase(key);
            }
        }

        void SanitizeActualJson(nlohmann::json& json)
        {
            for (auto& [key, value] : json.items())
            {
                if (value.is_object())
                {
                    SanitizeActualJson(value);
                    if (value.size() == 0)
                        json.erase(key);
                }
                else if (value.is_array())
                {
                    auto idx = 0;
                    for (auto& item : value)
                    {
                        if (item.is_object())
                            SanitizeActualJson(item);

                        if (item.size() == 0)
                            value.erase(idx);

                        ++idx;
                    }

                    if (value.size() == 0)
                        json.erase(key);
                }
                else if (key == "line")
                    json.erase(key);
            }
        }

        struct BroadcastListener
        {
            BroadcastListener(std::filesystem::path ndjsonin, std::filesystem::path expectedndjson, std::filesystem::path ndout, cucumber_cpp::library::util::Broadcaster& broadcaster)
                : listener(broadcaster, [this](const cucumber::messages::envelope& envelope)
                      {
                          OnEvent(envelope);
                      })
                , ndjsonin{ std::move(ndjsonin) }
                , expectedndjson{ std::move(expectedndjson) }
                , actualndjson(std::move(ndout))
            {
                while (!ifs.eof())
                {
                    std::string line;
                    std::getline(ifs, line);

                    if (line.empty())
                        continue;

                    auto json = nlohmann::json::parse(line);

                    if (json.contains("meta"))
                        continue;

                    SanitizeExpectedJson(json);

                    expectedEnvelopes.emplace_back(std::move(json));
                }
            }

            void OnEvent(const cucumber::messages::envelope& envelope)
            {
                nlohmann::json actualJson{};
                to_json(actualJson, envelope);

                if (expectedEnvelopes.empty())
                {
                    std::cerr << "Unexpected envelope: " << actualJson.dump() << "\n\n";
                    return;
                }

                SanitizeActualJson(actualJson);

                const auto expectedJson = expectedEnvelopes.front();
                expectedEnvelopes.pop_front();

                expectedOfs << expectedJson.dump() << "\n";
                actualOfs << actualJson.dump() << "\n";

                const auto expectedMessage = expectedJson.items().begin().key();

                const auto diff = nlohmann::json::diff(expectedJson, actualJson);

                EXPECT_THAT(actualJson, testing::Eq(expectedJson));
                // if (actualJson.contains(expectedMessage))
                // {
                //     if (actualJson[expectedMessage] == expectedJson[expectedMessage])
                //     {
                //         std::cout << "matching!!!! " << expectedMessage << "\n\n";
                //     }
                //     else
                //     {
                //         std::cerr << std::format("Mismatch {}: {}\n", expectedMessage, diff.dump());
                //         std::cerr << "expected: " << expectedJson[expectedMessage] << "\n";
                //         std::cerr << "actual  : " << actualJson[expectedMessage] << "\n\n";
                //     }
                // }
                // else
                // {
                //     std::cerr << std::format("Missing {}: {}\n", expectedMessage, diff.dump());
                // }
            }

        private:
            cucumber_cpp::library::util::Listener listener;
            std::filesystem::path ndjsonin;
            std::ifstream ifs{ ndjsonin };

            std::filesystem::path expectedndjson;
            std::ofstream expectedOfs{ expectedndjson };

            std::filesystem::path actualndjson;
            std::ofstream actualOfs{ actualndjson };

            std::list<nlohmann::json> expectedEnvelopes;
        };

        bool IsFeatureFile(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }

        std::vector<std::filesystem::path> GetFeatureFiles(std::vector<std::filesystem::path> paths)
        {
            std::vector<std::filesystem::path> files;

            for (const auto feature : paths)
                if (std::filesystem::is_directory(feature))
                    for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(IsFeatureFile))
                        files.emplace_back(entry.path());
                else
                    files.emplace_back(feature);

            return files;
        }

        struct StopwatchIncremental : cucumber_cpp::library::support::Stopwatch
        {
            virtual ~StopwatchIncremental() = default;

            void Start() override
            {
            }

            std::chrono::nanoseconds Duration() override
            {
                return current;
            }

            std::chrono::nanoseconds current{ std::chrono::milliseconds{ 1 } };
        };

        struct TimestampGeneratorIncremental : cucumber_cpp::library::support::TimestampGenerator
        {
            virtual ~TimestampGeneratorIncremental() = default;

            std::chrono::milliseconds Now() override
            {
                return current++;
            }

            std::chrono::milliseconds current{ 0 };
        };

        void RunDevkit(Devkit devkit)
        {
            devkit.paths = GetFeatureFiles(devkit.paths);

            cucumber_cpp::library::support::RunOptions runOptions{
                .sources = {
                    .paths = devkit.paths,
                    .tagExpression = devkit.tagExpression,
                },
                .runtime = {
                    .retry = devkit.retry,
                },
            };

            cucumber_cpp::library::cucumber_expression::ParameterRegistry parameterRegistry{};

            auto contextStorageFactory{ std::make_shared<cucumber_cpp::library::ContextStorageFactoryImpl>() };
            auto programContext{ std::make_unique<cucumber_cpp::Context>(contextStorageFactory) };

            cucumber_cpp::library::util::Broadcaster broadcaster;

            BroadcastListener broadcastListener{ devkit.ndjsonFile, devkit.ndjsonFile.parent_path() / "expected.ndjson", devkit.ndjsonFile.parent_path() / "actual.ndjson", broadcaster };

            cucumber_cpp::library::api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster);
            // EXPECT_NONFATAL_FAILURE(cucumber_cpp::library::api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster), "");
            // EXPECT_FATAL_FAILURE(cucumber_cpp::library::api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster), "");
        }
    }
}

TEST(CompatibilityTest, KIT_NAME)
{
    compatibility::StopwatchIncremental stopwatch;
    compatibility::TimestampGeneratorIncremental timestampGenerator;
    compatibility::RunDevkit(compatibility::LoadDevkit());
}
