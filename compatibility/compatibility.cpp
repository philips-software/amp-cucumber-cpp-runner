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
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <memory>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <utility>

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
            std::set<std::filesystem::path> paths;
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

        void RemoveIncompatibilities(nlohmann::json& json)
        {
            for (auto jsonIter = json.begin(); jsonIter != json.end();)
            {
                auto& key = jsonIter.key();
                auto& value = jsonIter.value();

                if (key == "exception")
                    jsonIter = json.erase(jsonIter);
                else if (key == "message")
                    jsonIter = json.erase(jsonIter);
                else if (key == "line")
                    jsonIter = json.erase(jsonIter);
                else if (key == "snippets")
                    jsonIter = json.erase(jsonIter);
                else if (value.is_object())
                {
                    RemoveIncompatibilities(value);
                    ++jsonIter;
                }
                else if (value.is_array())
                {
                    auto idx = 0;
                    for (auto valueIter = value.begin(); valueIter != value.end();)
                    {
                        auto& item = *valueIter;

                        if (item.is_object())
                            RemoveIncompatibilities(item);

                        ++valueIter;
                    }

                    ++jsonIter;
                }
                else if (key == "uri")
                {
                    auto uri = value.get<std::string>();

                    uri = std::regex_replace(uri, std::regex(R"(samples\/[^\/]+)"), KIT_FOLDER);
                    uri = std::regex_replace(uri, std::regex(R"(\.ts$)"), ".cpp");

                    json[key] = std::filesystem::path{ uri }.string();

                    ++jsonIter;
                }
                else
                    ++jsonIter;
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

                    expectedEnvelopes.emplace_back(std::move(json));
                }
            }

            void OnEvent(const cucumber::messages::envelope& envelope)
            {
                nlohmann::json actualJson{};
                to_json(actualJson, envelope);

                actualEnvelopes.emplace_back(std::move(actualJson));
            }

            void CompareEnvelopes()
            {
                EXPECT_THAT(actualEnvelopes.size(), testing::Eq(expectedEnvelopes.size()));

                for (auto& json : actualEnvelopes)
                {
                    RemoveIncompatibilities(json);
                    actualOfs << json.dump() << "\n";
                }

                for (auto& json : expectedEnvelopes)
                {
                    RemoveIncompatibilities(json);
                    expectedOfs << json.dump() << "\n";
                }

                while (!actualEnvelopes.empty() && !expectedEnvelopes.empty())
                {
                    auto actualJson = actualEnvelopes.front();
                    actualEnvelopes.pop_front();

                    auto expectedJson = expectedEnvelopes.front();
                    expectedEnvelopes.pop_front();

                    EXPECT_THAT(actualJson, testing::Eq(expectedJson));
                }
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
            std::list<nlohmann::json> actualEnvelopes;
        };

        bool IsFeatureFile(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }

        std::set<std::filesystem::path> GetFeatureFiles(std::set<std::filesystem::path> paths)
        {
            std::set<std::filesystem::path> files;

            for (const auto feature : paths)
                if (std::filesystem::is_directory(feature))
                    for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(IsFeatureFile))
                    {
                        std::cout << " found feature file: " << entry.path() << "\n";
                        files.insert(entry.path());
                    }
                else
                    files.insert(feature);

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
            const auto isReversed = std::string{ KIT_STRING }.ends_with("-reversed");

            cucumber_cpp::library::support::RunOptions runOptions{
                .sources = {
                    .paths = devkit.paths,
                    .tagExpression = devkit.tagExpression,
                    .ordering = isReversed ? cucumber_cpp::library::support::RunOptions::Ordering::reverse : cucumber_cpp::library::support::RunOptions::Ordering::defined,
                },
                .runtime = {
                    .retry = std::string{ KIT_STRING }.starts_with("retry") ? 2u : 0u,
                    .strict = true,
                },
            };

            cucumber_cpp::library::cucumber_expression::ParameterRegistry parameterRegistry{};

            auto contextStorageFactory{ std::make_shared<cucumber_cpp::library::ContextStorageFactoryImpl>() };
            auto programContext{ std::make_unique<cucumber_cpp::Context>(contextStorageFactory) };

            cucumber_cpp::library::util::Broadcaster broadcaster;

            BroadcastListener broadcastListener{ devkit.ndjsonFile, devkit.ndjsonFile.parent_path() / "expected.ndjson", devkit.ndjsonFile.parent_path() / "actual.ndjson", broadcaster };

            cucumber_cpp::library::api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster);

            broadcastListener.CompareEnvelopes();
        }
    }
}

TEST(CompatibilityTest, KIT_NAME)
{
#ifdef SKIP_TEST
    GTEST_SKIP();
#endif

    compatibility::StopwatchIncremental stopwatch;
    compatibility::TimestampGeneratorIncremental timestampGenerator;
    compatibility::RunDevkit(compatibility::LoadDevkit());
}
