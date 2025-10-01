#include "yaml-cpp/node/emit.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    struct TestErrors : testing::Test
    {
        TestErrors(std::string_view expression, std::string_view error)
            : expression{ expression }
            , error{ error }
        {}

        void TestBody() override
        {
        }

        std::string expression;
        std::string error;
    };

    namespace
    {
        std::vector<std::pair<std::string, YAML::Node>> GetTestData(const std::filesystem::path& path)
        {
            std::vector<std::pair<std::string, YAML::Node>> testdata;

            if (std::filesystem::is_regular_file(path) && (path.extension() == ".yml" || path.extension() == ".yaml"))
                testdata.emplace_back(path.string(), YAML::LoadFile(path.string()));
            else
                for (const auto& file : std::filesystem::directory_iterator(path))
                    if (file.is_regular_file() && (file.path().extension() == ".yml" || file.path().extension() == ".yaml"))
                        testdata.emplace_back(file.path().string(), YAML::LoadFile(file.path().string()));

            return testdata;
        }

        std::vector<testing::TestInfo*> RegisterMyTests()
        {
            std::vector<testing::TestInfo*> tests;
            std::filesystem::path testdataPath = "testdata/tag-expression/errors.yml";

            std::size_t lineNumber = 2;

            for (const auto& [file, testdata] : GetTestData(testdataPath))
            {
                for (const auto& test : testdata)
                {
                    auto factory = [node = test]() -> TestErrors*
                    {
                        return new TestErrors(node["expression"].as<std::string>(), node["error"].as<std::string>());
                    };

                    auto* testInfo = testing::RegisterTest("TestErrors", std::format("Test_{}_{}", tests.size(), lineNumber).c_str(), nullptr, nullptr, testdataPath.c_str(), lineNumber, factory);

                    tests.push_back(testInfo);

                    lineNumber += 2;
                }
            }

            return tests;
        }
    }

    auto testErrors = RegisterMyTests();
}
