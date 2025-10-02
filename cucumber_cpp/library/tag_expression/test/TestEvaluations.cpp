#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "yaml-cpp/node/emit.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include <cstdlib>
#include <filesystem>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    struct TestEvaluationsFixture : testing::Test
    {
    };

    struct TestEvaluations : TestEvaluationsFixture
    {
        TestEvaluations(std::string_view expression, YAML::Node variables, bool result)
            : expression{ expression }
            , variables{ variables }
            , result{ result }
        {}

        void TestBody() override
        {
            const auto tagExpression = Parse(expression);
            ASSERT_THAT(tagExpression, testing::NotNull());

            std::set<std::string, std::less<>> tags;
            for (const auto& var : variables)
                tags.insert(var.as<std::string>());

            EXPECT_THAT(tagExpression->Evaluate(tags), testing::Eq(result));
        }

    private:
        std::string expression;
        YAML::Node variables;
        bool result;
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
            std::filesystem::path testdataPath = "testdata/tag-expression/evaluations.yml";

            std::size_t lineNumber = 4;

            for (const auto& [file, testdata] : GetTestData(testdataPath))
            {
                for (const auto& node : testdata)
                {
                    for (const auto& test : node["tests"])
                    {
                        auto factory = [node = node, test = test]() -> TestEvaluationsFixture*
                        {
                            return new TestEvaluations(node["expression"].as<std::string>(), test["variables"], test["result"].as<bool>());
                        };

                        auto* testInfo = testing::RegisterTest("TestEvaluations", std::format("Test_{}_{}", tests.size(), lineNumber).c_str(), nullptr, nullptr, testdataPath.string().c_str(), lineNumber, factory);

                        tests.push_back(testInfo);

                        lineNumber += 2;
                    }

                    lineNumber += 2;
                }
            }

            return tests;
        }
    }

    auto testEvaluations = RegisterMyTests();
}
