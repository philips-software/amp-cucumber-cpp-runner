
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

    struct TestExpression : testing::Test
    {
        ParameterRegistry parameterRegistry{};

        std::optional<std::vector<std::any>> Match(std::string expr, std::string text)
        {
            Expression expression{ std::move(expr), parameterRegistry };
            return expression.Match(std::move(text));
        }
    };

    TEST_F(TestExpression, TestFromFiles)
    {
        std::filesystem::path testdataPath = "testdata/cucumber-expression/matching";

        for (const auto& [file, testdata] : GetTestData(testdataPath))
        {
            if (testdata["exception"])
                ASSERT_ANY_THROW(Match(testdata["expression"].as<std::string>(), testdata["text"].as<std::string>()))
                    << std::format("Test failed for file: {}", file);
            else
            {
                if (testdata["expected_args"].IsNull())
                    ASSERT_THAT(Match(testdata["expression"].as<std::string>(), testdata["text"].as<std::string>()), testing::IsFalse());
                else
                {
                    const auto expression = Expression{ testdata["expression"].as<std::string>(), parameterRegistry };
                    const auto matchOpt = expression.Match(testdata["text"].as<std::string>());

                    ASSERT_THAT(matchOpt, testing::IsTrue()) << FormatMessage(testdata, expression);

                    const auto match = *matchOpt;
                    for (std::size_t i = 0; i < testdata["expected_args"].size(); ++i)
                    {
                        if (match[i].type() == typeid(std::string))
                            ASSERT_THAT(std::any_cast<std::string>(match[i]), testdata["expected_args"][i].as<std::string>()) << FormatMessage(testdata, expression);
                        else if (match[i].type() == typeid(std::int32_t))
                            ASSERT_THAT(std::any_cast<std::int32_t>(match[i]), testdata["expected_args"][i].as<std::int32_t>()) << FormatMessage(testdata, expression);
                        else if (match[i].type() == typeid(std::int64_t))
                            ASSERT_THAT(std::any_cast<std::int64_t>(match[i]), testdata["expected_args"][i].as<std::int64_t>()) << FormatMessage(testdata, expression);
                        else if (match[i].type() == typeid(float))
                            ASSERT_THAT(std::any_cast<float>(match[i]), testdata["expected_args"][i].as<float>()) << FormatMessage(testdata, expression);
                        else if (match[i].type() == typeid(double))
                            ASSERT_THAT(std::any_cast<double>(match[i]), testdata["expected_args"][i].as<double>()) << FormatMessage(testdata, expression);
                        else
                            FAIL() << "Unknown type: " << match[i].type().name() << " for:\n"
                                   << FormatMessage(testdata, expression);
                    }
                }
            }
        }
    }

    TEST_F(TestExpression, MatchFloat)
    {
        EXPECT_THAT(Match(R"__({float})__", R"__()__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(.)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(,)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(-)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(E)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(,1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1.)__"), testing::IsFalse());

        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1)__"))[0]), testing::FloatNear(1.0f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-1)__"))[0]), testing::FloatNear(-1.0f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1.1)__"))[0]), testing::FloatNear(1.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,0)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000.1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,10)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,0.1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,000.1)__"), testing::IsFalse());
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-1.1)__"))[0]), testing::FloatNear(-1.1f, std::numeric_limits<float>::epsilon()));

        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(.1)__"))[0]), testing::FloatNear(0.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1)__"))[0]), testing::FloatNear(-0.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1000001)__"))[0]), testing::FloatNear(-0.1000001f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1E1)__"))[0]), testing::FloatNear(10.0, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(.1E1)__"))[0]), testing::FloatNear(1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(Match(R"__({float})__", R"__(1,E1)__"), testing::IsFalse());
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.01)__"))[0]), testing::FloatNear(-0.01, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E-1)__"))[0]), testing::FloatNear(-0.01, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E-2)__"))[0]), testing::FloatNear(-0.001, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E+1)__"))[0]), testing::FloatNear(-1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E+2)__"))[0]), testing::FloatNear(-10, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E1)__"))[0]), testing::FloatNear(-1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E2)__"))[0]), testing::FloatNear(-10, std::numeric_limits<float>::epsilon()));
    }

    TEST_F(TestExpression, FloatWithZero)
    {
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(0)__"))[0]), testing::FloatNear(0.0f, std::numeric_limits<float>::epsilon()));
    }

    TEST_F(TestExpression, MatchAnonymous)
    {
        EXPECT_THAT(std::any_cast<std::string>((*Match(R"__({})__", R"__(0.22)__"))[0]), testing::StrEq("0.22"));
    }

    TEST_F(TestExpression, MatchCustom)
    {
        struct CustomType
        {
            std::optional<std::string> text;
            std::optional<std::int64_t> number;
        };

        parameterRegistry.AddParameter("textAndOrNumber", { R"(([A-Z]+)?(?: )?([0-9]+)?)" }, [](MatchRange matches) -> std::any
            {
                std::optional<std::string> text{ matches[1].matched ? StringTo<std::string>(matches[1].str()) : std::optional<std::string>{ std::nullopt } };
                std::optional<std::int64_t> number{ matches[2].matched ? StringTo<std::int64_t>(matches[2].str()) : std::optional<std::int64_t>{ std::nullopt } };

                return CustomType{ text, number };
            });

        auto matchString{ Match(R"__({textAndOrNumber})__", R"__(ABC)__") };
        EXPECT_THAT(matchString, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchString)[0]).text.value(), testing::StrEq("ABC"));
        EXPECT_THAT(std::any_cast<CustomType>((*matchString)[0]).number, testing::IsFalse());

        auto matchInt{ Match(R"__({textAndOrNumber})__", R"__(123)__") };
        EXPECT_THAT(matchInt, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchInt)[0]).text, testing::IsFalse());
        EXPECT_THAT(std::any_cast<CustomType>((*matchInt)[0]).number.value(), testing::Eq(123));

        auto matchStringAndInt{ Match(R"__({textAndOrNumber})__", R"__(ABC 123)__") };
        EXPECT_THAT(matchStringAndInt, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchStringAndInt)[0]).text.value(), testing::StrEq("ABC"));
        EXPECT_THAT(std::any_cast<CustomType>((*matchStringAndInt)[0]).number.value(), testing::Eq(123));
    }

    TEST_F(TestExpression, ExposeSource)
    {
        auto expr = "I have {int} cuke(s)";
        Expression expression{ expr, parameterRegistry };
        EXPECT_THAT(expr, testing::StrEq(expression.Source()));
    }

    TEST_F(TestExpression, MatchBoolean)
    {
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(true)__"))[0]), testing::IsTrue());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(1)__"))[0]), testing::IsTrue());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(yes)__"))[0]), testing::IsTrue());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(on)__"))[0]), testing::IsTrue());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(enabled)__"))[0]), testing::IsTrue());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(active)__"))[0]), testing::IsTrue());

        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(false)__"))[0]), testing::IsFalse());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(0)__"))[0]), testing::IsFalse());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(2)__"))[0]), testing::IsFalse());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(off)__"))[0]), testing::IsFalse());
        EXPECT_THAT(std::any_cast<bool>((*Match(R"__({bool})__", R"__(foo)__"))[0]), testing::IsFalse());
    }

    TEST_F(TestExpression, ThrowUnknownParameterType)
    {
        auto expr = "I have {doesnotexist} cuke(s)";

        try
        {
            Expression expression{ expr, parameterRegistry };
            FAIL() << "Expected UndefinedParameterTypeError to be thrown";
        }
        catch (UndefinedParameterTypeError e)
        {
            EXPECT_THAT(e.what(), testing::StrEq("This Cucumber Expression has a problem at column 8:\n"
                                                 "\n"
                                                 "I have {doesnotexist} cuke(s)\n"
                                                 "       ^------------^\n"
                                                 "Undefined parameter type 'doesnotexist'\n"
                                                 "Please register a ParameterType for 'doesnotexist'\n"));
        }
    }
}
