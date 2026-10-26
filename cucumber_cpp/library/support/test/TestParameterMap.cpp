#include "cucumber_cpp/library/support/ParameterMap.hpp"
#include <gmock/gmock.h>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef CCR_HAS_RE2
#include <re2/re2.h>
#endif

namespace cucumber_cpp::library::support
{
    using testing::StrEq;

    namespace
    {
        void ExpectFullMatchOnlyKeys(const std::string& regex, const std::vector<std::string_view>& keys, const std::vector<std::string_view>& nonKeys)
        {
            const std::regex stdRegex{ regex };
            for (const auto key : keys)
            {
                std::smatch match;
                const std::string text{ key };
                EXPECT_TRUE(std::regex_match(text, match, stdRegex)) << "std::regex: " << key;
                EXPECT_THAT(match.str(1), StrEq(text));
            }
            for (const auto nonKey : nonKeys)
                EXPECT_FALSE(std::regex_match(std::string{ nonKey }, stdRegex)) << "std::regex: " << nonKey;

#ifdef CCR_HAS_RE2
            const RE2 re2Regex{ regex };
            ASSERT_TRUE(re2Regex.ok()) << re2Regex.error();
            for (const auto key : keys)
            {
                std::string captured;
                EXPECT_TRUE(RE2::FullMatch(key, re2Regex, &captured)) << "RE2: " << key;
                EXPECT_THAT(captured, StrEq(std::string{ key }));
            }
            for (const auto nonKey : nonKeys)
                EXPECT_FALSE(RE2::FullMatch(nonKey, re2Regex)) << "RE2: " << nonKey;
#endif
        }
    }

    TEST(ParameterMap, EscapeRegex_leaves_plain_text_untouched)
    {
        EXPECT_THAT(EscapeRegex("connected"), StrEq("connected"));
        EXPECT_THAT(EscapeRegex("stand by"), StrEq("stand by"));
        EXPECT_THAT(EscapeRegex(""), StrEq(""));
    }

    TEST(ParameterMap, EscapeRegex_escapes_all_special_characters)
    {
        EXPECT_THAT(EscapeRegex(R"(\^$.|?*+()[]{})"), StrEq(R"(\\\^\$\.\|\?\*\+\(\)\[\]\{\})"));
        EXPECT_THAT(EscapeRegex("c++"), StrEq(R"(c\+\+)"));
        EXPECT_THAT(EscapeRegex("a.b"), StrEq(R"(a\.b)"));
    }

    TEST(ParameterMap, KeysToRegex_single_key)
    {
        EXPECT_THAT(KeysToRegex({ "connected" }), StrEq("(connected)"));
    }

    TEST(ParameterMap, KeysToRegex_multiple_keys)
    {
        EXPECT_THAT(KeysToRegex({ "aaa", "bbb", "ccc" }), StrEq("(aaa|bbb|ccc)"));
    }

    TEST(ParameterMap, KeysToRegex_orders_longest_first_and_keeps_declaration_order_for_equal_lengths)
    {
        EXPECT_THAT(KeysToRegex({ "on", "one", "b", "a", "only" }), StrEq("(only|one|on|b|a)"));
    }

    TEST(ParameterMap, KeysToRegex_escapes_keys)
    {
        EXPECT_THAT(KeysToRegex({ "c++", "a.b", "(x)" }), StrEq(R"((c\+\+|a\.b|\(x\)))"));
    }

    TEST(ParameterMap, KeysToRegex_throws_on_empty_keys)
    {
        EXPECT_THROW((void)KeysToRegex({}), std::invalid_argument);
    }

    TEST(ParameterMap, KeysToRegex_throws_on_empty_key)
    {
        EXPECT_THROW((void)KeysToRegex({ "a", "" }), std::invalid_argument);
    }

    TEST(ParameterMap, KeysToRegex_throws_on_duplicate_key)
    {
        EXPECT_THROW((void)KeysToRegex({ "a", "b", "a" }), std::invalid_argument);
    }

    TEST(ParameterMap, KeysToRegex_duplicate_key_error_mentions_key)
    {
        try
        {
            (void)KeysToRegex({ "standby", "standby" });
            FAIL() << "expected std::invalid_argument";
        }
        catch (const std::invalid_argument& e)
        {
            EXPECT_THAT(e.what(), testing::HasSubstr("standby"));
        }
    }

    TEST(ParameterMap, KeysToRegex_generated_regex_matches_exactly_the_keys)
    {
        ExpectFullMatchOnlyKeys(KeysToRegex({ "connected", "disconnected", "standby" }),
            { "connected", "disconnected", "standby" },
            { "Connected", "connect", "shutdown", "connectedstandby", "" });
    }

    TEST(ParameterMap, KeysToRegex_generated_regex_matches_special_character_keys_literally)
    {
        ExpectFullMatchOnlyKeys(KeysToRegex({ "c++", "a.b", "(x)", "[y]", "{z}", "a|b", R"(^$\?*)" }),
            { "c++", "a.b", "(x)", "[y]", "{z}", "a|b", R"(^$\?*)" },
            { "c", "cc", "axb", "x", "y", "z", "a", "b" });
    }

    TEST(ParameterMap, KeysToRegex_generated_regex_prefers_longest_key)
    {
        const std::regex regex{ KeysToRegex({ "on", "one" }) };
        std::smatch match;
        const std::string text{ "one" };
        ASSERT_TRUE(std::regex_search(text, match, regex));
        EXPECT_THAT(match.str(1), StrEq("one"));
    }
}
