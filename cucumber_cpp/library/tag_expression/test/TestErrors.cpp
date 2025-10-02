#include "cucumber_cpp/library/tag_expression/TagExpressionError.hpp"
#include "cucumber_cpp/library/tag_expression/TagExpressionParser.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <string_view>

namespace cucumber_cpp::library::tag_expression
{
    struct TestErrors : testing::Test
    {
        void ExpectError(std::string_view expression, std::string_view error)
        {
            ASSERT_THROW(try { TagExpressionParser{}.Parse(expression); } catch (const TagExpressionError& ex) {
                std::cout << "expected: " << error << "\n";
                std::cout << "actual  : " << ex.what() << "\n";
                EXPECT_THAT(ex.what(), testing::HasSubstr(error));
                throw; }, TagExpressionError);
        }
    };

    TEST_F(TestErrors, ParseFailsWithUnbalancedParens)
    {
        ExpectError("( a and b ))", "Missing '(': Too few open-parens");
        ExpectError("( ( a and b )", "Unclosed '(': Too many open-parens");
    }

    TEST_F(TestErrors, ParseFailsWithMissingOperationArgs)
    {
        ExpectError("a not ( and )", "Syntax error. Expected operator after a");
    }

    TEST_F(TestErrors, ParseFailsWithOnlyOperations)
    {
        ExpectError("or or", "Syntax error. Expected operand after BEGIN");
    }

    TEST_F(TestErrors, ParseFailsForArgsWithoutOperation)
    {
        ExpectError("a b", "Syntax error. Expected operator after a");
    }

    TEST_F(TestErrors, ParseFailsForEmptyParensGroups)
    {
        ExpectError("(())", "Syntax error. Expected operand after (");
        ExpectError("(() ())", "Syntax error. Expected operand after (");
    }

    TEST_F(TestErrors, ParseFailsWithRpnNotation)
    {
        ExpectError("a b or", "Syntax error. Expected operator after a");
        ExpectError("a and (b not)", "Syntax error. Expected operator after b");
        ExpectError("a and (b c) or", "Syntax error. Expected operator after b");
    }

    TEST_F(TestErrors, ParseFailsAndOperationWithTooFewArgs)
    {
        ExpectError("a and ", "and: Too few operands");
        ExpectError("  and b", "Syntax error. Expected operand after BEGIN");
    }

    TEST_F(TestErrors, ParseFailsOrOperationWithTooFewArgs)
    {
        ExpectError("a or ", "or: Too few operands");
        ExpectError("  or b", "Syntax error. Expected operand after BEGIN");
        ExpectError("a and b or ", "or: Too few operands");
    }

    TEST_F(TestErrors, ParseFailsNotOperationWithTooFewArgs)
    {
        ExpectError("not ", "not: Too few operands");
        ExpectError("not ()", "Syntax error. Expected operand after (");
        ExpectError("not () and b", "Syntax error. Expected operand after (");
        ExpectError("not () or b", "Syntax error. Expected operand after (");
    }

}
