#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionTokenizer.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include "gmock/gmock.h"
#include <cstddef>
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        const std::map<std::string_view, TokenType> tokenTypeMap = {
            { "START_OF_LINE", TokenType::startOfLine },
            { "END_OF_LINE", TokenType::endOfLine },
            { "TEXT", TokenType::text },
            { "WHITE_SPACE", TokenType::whiteSpace },
            { "BEGIN_PARAMETER", TokenType::beginParameter },
            { "END_PARAMETER", TokenType::endParameter },
            { "BEGIN_OPTIONAL", TokenType::beginOptional },
            { "END_OPTIONAL", TokenType::endOptional },
            { "ALTERNATION", TokenType::alternation }
        };

        std::vector<Token> CreateTokens(const YAML::Node& node)
        {
            std::vector<Token> tokens;
            tokens.reserve(node.size());
            for (const auto& expectedToken : node)
                tokens.emplace_back(
                    tokenTypeMap.at(expectedToken["type"].as<std::string>()),
                    expectedToken["text"].as<std::string>(),
                    expectedToken["start"].as<std::size_t>(),
                    expectedToken["end"].as<std::size_t>());
            return tokens;
        }

        std::vector<std::pair<std::string, YAML::Node>> GetTestData(const std::filesystem::path& path)
        {
            std::vector<std::pair<std::string, YAML::Node>> testdata;

            for (const auto& file : std::filesystem::directory_iterator(path))
                if (file.is_regular_file() && file.path().extension() == ".yaml")
                    testdata.emplace_back(file.path().string(), YAML::LoadFile(file.path().string()));

            return testdata;
        }
    }

    TEST(TestExpressionTokenizer, TestFromFiles)
    {
        std::filesystem::path testdataPath = "testdata/cucumber-expression/tokenizer";

        for (const auto& [file, testdata] : GetTestData(testdataPath))
            if (testdata["exception"])
                ASSERT_ANY_THROW(ExpressionTokenizer{}.Tokenize(testdata["expression"].as<std::string>()))
                    << std::format("Test failed for file: {}", file);
            else
            {
                const auto actual = ExpressionTokenizer{}.Tokenize(testdata["expression"].as<std::string>());
                const auto expected = CreateTokens(testdata["expected_tokens"]);
                ASSERT_THAT(actual, testing::ElementsAreArray(expected))
                    << std::format("Test failed for file: {}", file);
            }
    }

    TEST(TestExpressionTokenizer, TestNameOf)
    {
        EXPECT_THAT(Token::NameOf(TokenType::startOfLine), testing::StrEq("startOfLine"));
        EXPECT_THAT(Token::NameOf(TokenType::endOfLine), testing::StrEq("endOfLine"));
        EXPECT_THAT(Token::NameOf(TokenType::whiteSpace), testing::StrEq("whiteSpace"));
        EXPECT_THAT(Token::NameOf(TokenType::beginOptional), testing::StrEq("beginOptional"));
        EXPECT_THAT(Token::NameOf(TokenType::endOptional), testing::StrEq("endOptional"));
        EXPECT_THAT(Token::NameOf(TokenType::beginParameter), testing::StrEq("beginParameter"));
        EXPECT_THAT(Token::NameOf(TokenType::endParameter), testing::StrEq("endParameter"));
        EXPECT_THAT(Token::NameOf(TokenType::alternation), testing::StrEq("alternation"));
        EXPECT_THAT(Token::NameOf(TokenType::text), testing::StrEq("text"));
    }
}
