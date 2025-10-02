#include "cucumber_cpp/library/tag_expression/TagExpressionParser.hpp"
#include "yaml-cpp/node/emit.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    struct TestParsingFixture : testing::Test
    {
        TestParsingFixture(std::string_view expression, std::string_view formatted)
            : expression{ expression }
            , formatted{ formatted }
        {}

        void TestBody() override
        {
            const auto tagExpression = TagExpressionParser{}.Parse(expression);

            ASSERT_THAT(tagExpression, testing::NotNull());
            const auto actualText = static_cast<std::string>(*tagExpression);
            EXPECT_THAT(actualText, testing::StrEq(formatted));
        }

        std::string expression;
        std::string formatted;
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
            std::filesystem::path testdataPath = "testdata/tag-expression/parsing.yml";

            std::size_t lineNumber = 2;

            for (const auto& [file, testdata] : GetTestData(testdataPath))
            {
                for (const auto& node : testdata)
                {
                    auto factory = [node = node]() -> TestParsingFixture*
                    {
                        return new TestParsingFixture(node["expression"].as<std::string>(), node["formatted"].as<std::string>());
                    };

                    auto* testInfo = testing::RegisterTest("TestParsing", std::format("Test_{}_{}", tests.size(), lineNumber).c_str(), nullptr, nullptr, testdataPath.c_str(), lineNumber, factory);

                    tests.push_back(testInfo);

                    lineNumber += 2;
                }
            }

            return tests;
        }
    }

    auto TestParsing = RegisterMyTests();
}
