#include "BaseCompatibility.hpp"
#include "cucumber_cpp/Steps.hpp"
#include "cucumber_cpp/library/Application.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace compatibility
{
    namespace
    {
        void RemoveIncompatibilities(nlohmann::json& json, const std::string& sourceDir)
        {
            for (auto jsonIter = json.begin(); jsonIter != json.end();)
            {
                const auto& key = jsonIter.key();
                auto& value = jsonIter.value();

                if (key == "exception" || key == "message" || key == "line" || key == "snippets")
                    jsonIter = json.erase(jsonIter);
                else if (key == "timestamp" || key == "duration")
                    jsonIter = json.erase(jsonIter);
                else if (value.is_object())
                {
                    RemoveIncompatibilities(value, sourceDir);
                    ++jsonIter;
                }
                else if (value.is_array())
                {
                    for (auto& element : value)
                        if (element.is_object())
                            RemoveIncompatibilities(element, sourceDir);
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

                    uri = std::regex_replace(uri, std::regex(R"(samples\/[^\/]+)"), sourceDir);
                    uri = std::regex_replace(uri, std::regex(R"(\.ts$)"), ".cpp");

                    std::filesystem::path path{ uri };
                    if (path.is_relative())
                        path = std::filesystem::path{ CCR_BINARY_DIR } / path;

                    json[key] = std::filesystem::canonical(path).string();

                    ++jsonIter;
                }
                else
                    ++jsonIter;
            }
        }

        std::list<nlohmann::json> LoadNdjson(const std::filesystem::path& path)
        {
            std::list<nlohmann::json> envelopes;
            std::ifstream ifs{ path };
            std::string line;

            while (std::getline(ifs, line))
            {
                if (line.empty())
                    continue;

                auto json = nlohmann::json::parse(line);

                if (json.contains("meta"))
                    continue;

                envelopes.emplace_back(std::move(json));
            }

            return envelopes;
        }

        void CompareEnvelopes(std::list<nlohmann::json>& actual, std::list<nlohmann::json>& expected, const std::string& sourceDir, const std::filesystem::path& kitDir)
        {
            for (auto& json : actual)
                RemoveIncompatibilities(json, sourceDir);
            for (auto& json : expected)
                RemoveIncompatibilities(json, sourceDir);

            // Write out normalized envelopes for debugging
            {
                std::ofstream ofs{ kitDir / "actual.ndjson" };
                for (const auto& json : actual)
                    ofs << json.dump() << "\n";
            }
            {
                std::ofstream ofs{ kitDir / "expected.ndjson" };
                for (const auto& json : expected)
                    ofs << json.dump() << "\n";
            }

            EXPECT_THAT(actual.size(), testing::Eq(expected.size()));

            auto actualIter = actual.begin();
            auto expectedIter = expected.begin();

            while (actualIter != actual.end() && expectedIter != expected.end())
            {
                EXPECT_THAT(*actualIter, testing::Eq(*expectedIter));
                ++actualIter;
                ++expectedIter;
            }
        }
    }

    void RunDevkit(const KitInfo& kit)
    {
        const auto actualNdjsonPath = kit.sourceDir / "actual_run.ndjson";

        // Build CLI args: program name, --load <plugin>, --format message:<output>, extra args, -- <feature-dir>
        std::vector<std::string> argStrings;
        argStrings.emplace_back("compatibility.test");
        argStrings.emplace_back("--load");
        argStrings.emplace_back(kit.pluginPath.string());
        argStrings.emplace_back("--format");
        argStrings.emplace_back("message:" + actualNdjsonPath.string());

        for (const auto& arg : kit.extraArgs)
            argStrings.emplace_back(arg);

        argStrings.emplace_back("--no-recursive");
        argStrings.emplace_back(kit.sourceDir.string());

        std::vector<const char*> argv;
        argv.reserve(argStrings.size());
        for (const auto& s : argStrings)
            argv.push_back(s.c_str());

        {
            cucumber_cpp::library::Application app{ std::make_shared<cucumber_cpp::library::ContextStorageFactoryImpl>(), false };
            static_cast<void>(app.Run(static_cast<int>(argv.size()), argv.data()));
        }

        auto actualEnvelopes = LoadNdjson(actualNdjsonPath);
        auto expectedEnvelopes = LoadNdjson(kit.ndjsonFile);

        CompareEnvelopes(actualEnvelopes, expectedEnvelopes, kit.sourceDir.string(), kit.sourceDir);

        std::filesystem::remove(actualNdjsonPath);
    }
}
