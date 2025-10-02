
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include "gmock/gmock.h"
#include <any>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        std::vector<std::pair<std::string, YAML::Node>> GetTestData(const std::filesystem::path& path)
        {
            std::vector<std::pair<std::string, YAML::Node>> testdata;

            for (const auto& file : std::filesystem::directory_iterator(path))
                if (file.is_regular_file() && file.path().extension() == ".yaml")
                    testdata.emplace_back(file.path().string(), YAML::LoadFile(file.path().string()));

            return testdata;
        }

        std::string FormatMessage(const YAML::Node& node, const Expression& expression)
        {
            return std::format("failed to match {}\n"
                               "regex           {}\n"
                               "against         {}",
                node["expression"].as<std::string>(), expression.Pattern(), node["text"].as<std::string>());
        }
    }

    TEST(TestTransformation, TestFromFiles)
    {
        std::filesystem::path testdataPath = "testdata/cucumber-expression/transformation";
        ParameterRegistry parameterRegistry{};

        for (const auto& [file, testdata] : GetTestData(testdataPath))
        {
            const auto expression = Expression{ testdata["expression"].as<std::string>(), parameterRegistry };
            const auto actualRegex = expression.Pattern();

            EXPECT_THAT(actualRegex, testing::StrEq(testdata["expected_regex"].as<std::string>()))
                << std::format("Test failed for {}", testdata["expression"].as<std::string>());
        }
    }
}
