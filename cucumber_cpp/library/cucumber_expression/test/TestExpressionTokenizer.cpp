#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionTokenizer.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <stdexcept>

namespace cucumber_cpp::library::cucumber_expression
{
    struct TestExpressionTokenizer : testing::Test
    {};

    TEST_F(TestExpressionTokenizer, Construct)
    {
        ExpressionTokenizer tokenizer;
    }

    TEST_F(TestExpressionTokenizer, EmptyString)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize(""), testing::ElementsAre(
                                                Token{ TokenType::startOfLine, "", 0, 0 },
                                                Token{ TokenType::endOfLine, "", 0, 0 }));
    }

    TEST_F(TestExpressionTokenizer, Phrase)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("three blind mice"), testing::ElementsAre(
                                                                Token{ TokenType::startOfLine, "", 0, 0 },
                                                                Token{ TokenType::text, "three", 0, 5 },
                                                                Token{ TokenType::whiteSpace, " ", 5, 6 },
                                                                Token{ TokenType::text, "blind", 6, 11 },
                                                                Token{ TokenType::whiteSpace, " ", 11, 12 },
                                                                Token{ TokenType::text, "mice", 12, 16 },
                                                                Token{ TokenType::endOfLine, "", 16, 16 }));
    }

    TEST_F(TestExpressionTokenizer, EscapedSpace)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("\\ "), testing::ElementsAre(
                                                   Token{ TokenType::startOfLine, "", 0, 0 },
                                                   Token{ TokenType::text, " ", 0, 2 },
                                                   Token{ TokenType::endOfLine, "", 2, 2 }));
    }

    TEST_F(TestExpressionTokenizer, Parameter)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("{string}"), testing::ElementsAre(
                                                        Token{ TokenType::startOfLine, "", 0, 0 },
                                                        Token{ TokenType::beginParameter, "{", 0, 1 },
                                                        Token{ TokenType::text, "string", 1, 7 },
                                                        Token{ TokenType::endParameter, "}", 7, 8 },
                                                        Token{ TokenType::endOfLine, "", 8, 8 }));
    }

    TEST_F(TestExpressionTokenizer, ParameterPhrase)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("three {string} mice"), testing::ElementsAre(
                                                                   Token{ TokenType::startOfLine, "", 0, 0 },
                                                                   Token{ TokenType::text, "three", 0, 5 },
                                                                   Token{ TokenType::whiteSpace, " ", 5, 6 },
                                                                   Token{ TokenType::beginParameter, "{", 6, 7 },
                                                                   Token{ TokenType::text, "string", 7, 13 },
                                                                   Token{ TokenType::endParameter, "}", 13, 14 },
                                                                   Token{ TokenType::whiteSpace, " ", 14, 15 },
                                                                   Token{ TokenType::text, "mice", 15, 19 },
                                                                   Token{ TokenType::endOfLine, "", 19, 19 }));
    }

    TEST_F(TestExpressionTokenizer, Optional)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("(blind)"), testing::ElementsAre(
                                                       Token{ TokenType::startOfLine, "", 0, 0 },
                                                       Token{ TokenType::beginOptional, "(", 0, 1 },
                                                       Token{ TokenType::text, "blind", 1, 6 },
                                                       Token{ TokenType::endOptional, ")", 6, 7 },
                                                       Token{ TokenType::endOfLine, "", 7, 7 }));
    }

    TEST_F(TestExpressionTokenizer, OptionalPhrase)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("three (blind) mice"), testing::ElementsAre(
                                                                  Token{ TokenType::startOfLine, "", 0, 0 },
                                                                  Token{ TokenType::text, "three", 0, 5 },
                                                                  Token{ TokenType::whiteSpace, " ", 5, 6 },
                                                                  Token{ TokenType::beginOptional, "(", 6, 7 },
                                                                  Token{ TokenType::text, "blind", 7, 12 },
                                                                  Token{ TokenType::endOptional, ")", 12, 13 },
                                                                  Token{ TokenType::whiteSpace, " ", 13, 14 },
                                                                  Token{ TokenType::text, "mice", 14, 18 },
                                                                  Token{ TokenType::endOfLine, "", 18, 18 }));
    }

    TEST_F(TestExpressionTokenizer, EscapedParameter)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("\\{string\\}"), testing::ElementsAre(
                                                            Token{ TokenType::startOfLine, "", 0, 0 },
                                                            Token{ TokenType::text, "{string}", 0, 10 },
                                                            Token{ TokenType::endOfLine, "", 10, 10 }));
    }

    TEST_F(TestExpressionTokenizer, EscapedOptional)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("\\(blind\\)"), testing::ElementsAre(
                                                           Token{ TokenType::startOfLine, "", 0, 0 },
                                                           Token{ TokenType::text, "(blind)", 0, 9 },
                                                           Token{ TokenType::endOfLine, "", 9, 9 }));
    }

    TEST_F(TestExpressionTokenizer, EscapedEndOfLine)
    {
        ExpressionTokenizer tokenizer;
        EXPECT_THROW(tokenizer.Tokenize("\\"), TheEndOfLineCannotBeEscaped);
    }

    TEST_F(TestExpressionTokenizer, EscapedCharHasStartIndexOfTextToken)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize(" \\/ "), testing::ElementsAre(
                                                     Token{ TokenType::startOfLine, "", 0, 0 },
                                                     Token{ TokenType::whiteSpace, " ", 0, 1 },
                                                     Token{ TokenType::text, "/", 1, 3 },
                                                     Token{ TokenType::whiteSpace, " ", 3, 4 },
                                                     Token{ TokenType::endOfLine, "", 4, 4 }));
    }

    TEST_F(TestExpressionTokenizer, EscapedAlternation)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("blind\\ and\\ famished\\/cripple mice"), testing::ElementsAre(
                                                                                     Token{ TokenType::startOfLine, "", 0, 0 },
                                                                                     Token{ TokenType::text, "blind and famished/cripple", 0, 29 },
                                                                                     Token{ TokenType::whiteSpace, " ", 29, 30 },
                                                                                     Token{ TokenType::text, "mice", 30, 34 },
                                                                                     Token{ TokenType::endOfLine, "", 34, 34 }));
    }

    TEST_F(TestExpressionTokenizer, EscapeNonReservedCharacter)
    {
        ExpressionTokenizer tokenizer;
        EXPECT_THROW(tokenizer.Tokenize("\\["), CantEscape);
    }

    TEST_F(TestExpressionTokenizer, Alternation)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("blind/cripple"), testing::ElementsAre(
                                                             Token{ TokenType::startOfLine, "", 0, 0 },
                                                             Token{ TokenType::text, "blind", 0, 5 },
                                                             Token{ TokenType::alternation, "/", 5, 6 },
                                                             Token{ TokenType::text, "cripple", 6, 13 },
                                                             Token{ TokenType::endOfLine, "", 13, 13 }));
    }

    TEST_F(TestExpressionTokenizer, AlternationPhrase)
    {
        ExpressionTokenizer tokenizer;
        ASSERT_THAT(tokenizer.Tokenize("three blind/cripple mice"), testing::ElementsAre(
                                                                        Token{ TokenType::startOfLine, "", 0, 0 },
                                                                        Token{ TokenType::text, "three", 0, 5 },
                                                                        Token{ TokenType::whiteSpace, " ", 5, 6 },
                                                                        Token{ TokenType::text, "blind", 6, 11 },
                                                                        Token{ TokenType::alternation, "/", 11, 12 },
                                                                        Token{ TokenType::text, "cripple", 12, 19 },
                                                                        Token{ TokenType::whiteSpace, " ", 19, 20 },
                                                                        Token{ TokenType::text, "mice", 20, 24 },
                                                                        Token{ TokenType::endOfLine, "", 24, 24 }));
    }
}
