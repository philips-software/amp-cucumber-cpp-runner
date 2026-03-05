#include "BaseCompatibility.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/Steps.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gmock/gmock.h>
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

namespace compatibility
{
    namespace
    {
        void RemoveIncompatibilities(nlohmann::json& json, const Devkit& devkit)
        {
            for (auto jsonIter = json.begin(); jsonIter != json.end();)
            {
                const auto& key = jsonIter.key();
                auto& value = jsonIter.value();

                if (key == "exception" || key == "message" || key == "line" || key == "snippets")
                    jsonIter = json.erase(jsonIter);
                else if (value.is_object())
                {
                    RemoveIncompatibilities(value, devkit);
                    ++jsonIter;
                }
                else if (value.is_array())
                {
                    for (auto valueIter = value.begin(); valueIter != value.end();)
                    {
                        if (valueIter->is_object())
                            RemoveIncompatibilities(*valueIter, devkit);

                        ++valueIter;
                    }

                    ++jsonIter;
                }
                else if (key == "data")
                {
                    json[key] = std::regex_replace(json[key].get<std::string>(), std::regex(R"(\r\n)"), "\n");
                    ++jsonIter;
                }
                else if (key == "uri")
                {
                    auto uri = value.get<std::string>();

                    uri = std::regex_replace(uri, std::regex(R"(samples\/[^\/]+)"), devkit.folder);
                    uri = std::regex_replace(uri, std::regex(R"(\.ts$)"), ".cpp");

                    json[key] = std::filesystem::canonical(uri).string();

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

            void CompareEnvelopes(const Devkit& devkit)
            {
                EXPECT_THAT(actualEnvelopes.size(), testing::Eq(expectedEnvelopes.size()));

                for (auto& json : actualEnvelopes)
                {
                    RemoveIncompatibilities(json, devkit);
                    actualOfs << json.dump() << "\n";
                }

                for (auto& json : expectedEnvelopes)
                {
                    RemoveIncompatibilities(json, devkit);
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

        std::set<std::filesystem::path, std::less<>> GetFeatureFiles(const std::set<std::filesystem::path, std::less<>>& paths)
        {
            std::set<std::filesystem::path, std::less<>> files;

            for (const auto& feature : paths)
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

        struct StopwatchIncremental : cucumber_cpp::library::util::Stopwatch
        {
            virtual ~StopwatchIncremental() = default;

            std::chrono::high_resolution_clock::time_point Start() override
            {
                return {};
            }

            std::chrono::nanoseconds Duration([[maybe_unused]] std::chrono::high_resolution_clock::time_point timePoint) override
            {
                return current;
            }

            std::chrono::nanoseconds current{ std::chrono::milliseconds{ 1 } };
        };

        struct TimestampGeneratorIncremental : cucumber_cpp::library::util::TimestampGenerator
        {
            virtual ~TimestampGeneratorIncremental() = default;

            std::chrono::milliseconds Now() override
            {
                return current++;
            }

            std::chrono::milliseconds current{ 0 };
        };
    }

    void RunDevkit(Devkit devkit)
    {
        compatibility::StopwatchIncremental stopwatch;
        compatibility::TimestampGeneratorIncremental timestampGenerator;

        devkit.paths = GetFeatureFiles(devkit.paths);
        const auto isReversed = devkit.kitString.ends_with("-reversed");

        cucumber_cpp::library::support::RunOptions runOptions{
            .sources = {
                .paths = devkit.paths,
                .tagExpression = cucumber_cpp::library::tag_expression::Parse(devkit.tagExpression),
                .ordering = isReversed ? cucumber_cpp::library::support::RunOptions::Ordering::reverse : cucumber_cpp::library::support::RunOptions::Ordering::defined,
            },
            .runtime = {
                .retry = devkit.kitString.starts_with("retry") ? 2u : 0u,
                .strict = true,
                .retryTagExpression = cucumber_cpp::library::tag_expression::Parse(""),
            },
        };

        cucumber_cpp::library::cucumber_expression::ParameterRegistry parameterRegistry{ cucumber_cpp::library::support::DefinitionRegistration::Instance().GetRegisteredParameters() };

        auto contextStorageFactory{ std::make_shared<cucumber_cpp::library::ContextStorageFactoryImpl>() };
        auto programContext{ std::make_unique<cucumber_cpp::Context>(contextStorageFactory) };

        cucumber_cpp::library::util::Broadcaster broadcaster;

        BroadcastListener broadcastListener{ devkit.ndjsonFile, devkit.ndjsonFile.parent_path() / "expected.ndjson", devkit.ndjsonFile.parent_path() / "actual.ndjson", broadcaster };

        cucumber_cpp::library::api::Formatters formatters;
        cucumber_cpp::library::api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster, formatters, { "junit", "message", "pretty", "summary", "usage" }, {});

        broadcastListener.CompareEnvelopes(devkit);
    }
}
