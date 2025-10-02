#include "cucumber_cpp/library/tag_expression/TagExpressionError.hpp"
#include "cucumber_cpp/library/tag_expression/TagExpressionParser.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <string_view>

namespace cucumber_cpp::library::tag_expression
{
    struct TestErrors : testing::Test
    {
        void ExpectErrror(std::string_view expression, std::string_view error)
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
        ExpectErrror("( a and b ))", "Missing '(': Too few open-parens");
        ExpectErrror("( ( a and b )", "Unclosed '(': Too many open-parens");
    }

    TEST_F(TestErrors, ParseFailsWithMissingOperationArgs)
    {
        ExpectErrror("a not ( and )", "Syntax error. Expected operator after a");
    }

    TEST_F(TestErrors, ParseFailsWithOnlyOperations)
    {
        ExpectErrror("or or", "Syntax error. Expected operand after BEGIN");
    }

    TEST_F(TestErrors, ParseFailsForArgsWithoutOperation)
    {
        ExpectErrror("a b", "Syntax error. Expected operator after a");
    }

    TEST_F(TestErrors, ParseFailsForEmptyParensGroups)
    {
        ExpectErrror("(())", "Syntax error. Expected operand after (");
        ExpectErrror("(() ())", "Syntax error. Expected operand after (");
    }

    TEST_F(TestErrors, ParseFailsWithRpnNotation)
    {
        ExpectErrror("a b or", "Syntax error. Expected operator after a");
        ExpectErrror("a and (b not)", "Syntax error. Expected operator after b");
        ExpectErrror("a and (b c) or", "Syntax error. Expected operator after b");
    }

    TEST_F(TestErrors, ParseFailsAndOperationWithTooFewArgs)
    {
        ExpectErrror("a and ", "and: Too few operands");
        ExpectErrror("  and b", "Syntax error. Expected operand after BEGIN");
    }

    TEST_F(TestErrors, ParseFailsOrOperationWithTooFewArgs)
    {
        ExpectErrror("a or ", "or: Too few operands");
        ExpectErrror("  or b", "Syntax error. Expected operand after BEGIN");
        ExpectErrror("a and b or ", "or: Too few operands");
    }

    TEST_F(TestErrors, ParseFailsNotOperationWithTooFewArgs)
    {
        ExpectErrror("not ", "not: Too few operands");
        ExpectErrror("not ()", "Syntax error. Expected operand after (");
        ExpectErrror("not () and b", "Syntax error. Expected operand after (");
        ExpectErrror("not () or b", "Syntax error. Expected operand after (");
    }

}
