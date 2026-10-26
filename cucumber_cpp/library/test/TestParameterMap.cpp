#include "cucumber/cucumber-expressions/Argument.hpp"
#include "cucumber/cucumber-expressions/Expression.hpp"
#include "cucumber/cucumber-expressions/ParameterRegistry.hpp"
#include "cucumber_cpp/library/ParameterMap.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include <algorithm>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <string>

namespace
{
    enum class TestState : std::uint8_t
    {
        connected,
        disconnected,
        standby,
    };

    struct TestVersion
    {
        int major{};
        int minor{};

        bool operator==(const TestVersion&) const = default;
    };

    constexpr int entries = 7;
}

PARAMETER_MAP(TestState, ("testParameterMapState", true), ({ "connected", TestState::connected }, { "disconnected", TestState::disconnected }, { "standby", TestState::standby }));

PARAMETER_MAP(int, ("testParameterMapNumber", false), ({ "on", 100 }, { "one", 1 }, { "c++", 3 }, { "a.b", 4 }));

PARAMETER_MAP(TestVersion, ("testParameterMapVersion", false), ({ "v1", TestVersion{ 1, 0 } }, { "v1.1", TestVersion{ 1, 1 } }));

PARAMETER_MAP(int, ("testParameterMapEntriesName", false), ({ "seven", entries }));

namespace cucumber_cpp::library
{
    using testing::Eq;
    using testing::IsFalse;
    using testing::IsTrue;
    using testing::StrEq;

    namespace
    {
        std::optional<cucumber::cucumber_expressions::CustomParameterEntry> FindParameter(const std::string& name)
        {
            const auto parameters = support::DefinitionRegistration::Instance().GetRegisteredParameters();
            const auto iter = std::ranges::find_if(parameters, [&name](const auto& entry)
                {
                    return entry.params.name == name;
                });

            if (iter == parameters.end())
                return std::nullopt;

            return *iter;
        }

        template<class T>
        T Match(const std::string& expression, const std::string& text)
        {
            cucumber::cucumber_expressions::ParameterRegistry registry{ support::DefinitionRegistration::Instance().GetRegisteredParameters() };
            const cucumber::cucumber_expressions::Expression expr{ expression, registry };
            const auto arguments = expr.MatchToArguments(text);
            if (!arguments || arguments->size() != 1)
                throw std::runtime_error{ "no single match for: " + text };
            return arguments->front().GetValue<T>();
        }

        bool Matches(const std::string& expression, const std::string& text)
        {
            cucumber::cucumber_expressions::ParameterRegistry registry{ support::DefinitionRegistration::Instance().GetRegisteredParameters() };
            const cucumber::cucumber_expressions::Expression expr{ expression, registry };
            return expr.MatchToArguments(text).has_value();
        }
    }

    TEST(ParameterMap, registers_parameter_with_generated_regex)
    {
        const auto state = FindParameter("testParameterMapState");
        ASSERT_THAT(state.has_value(), IsTrue());
        EXPECT_THAT(state->params.regex, StrEq("(disconnected|connected|standby)"));
        EXPECT_THAT(state->params.useForSnippets, IsTrue());

        const auto number = FindParameter("testParameterMapNumber");
        ASSERT_THAT(number.has_value(), IsTrue());
        EXPECT_THAT(number->params.regex, StrEq(R"((one|c\+\+|a\.b|on))"));
        EXPECT_THAT(number->params.useForSnippets, IsFalse());
    }

    TEST(ParameterMap, registered_converter_maps_keys_to_values)
    {
        const auto& converter = cucumber::cucumber_expressions::ConverterTypeMap<std::optional<TestState>>::Instance().At("testParameterMapState");

        EXPECT_THAT(converter({ "connected" }), Eq(TestState::connected));
        EXPECT_THAT(converter({ "disconnected" }), Eq(TestState::disconnected));
        EXPECT_THAT(converter({ "standby" }), Eq(TestState::standby));
    }

    TEST(ParameterMap, cucumber_expression_converts_to_enum)
    {
        EXPECT_THAT(Match<TestState>("the device is {testParameterMapState}", "the device is connected"), Eq(TestState::connected));
        EXPECT_THAT(Match<TestState>("the device is {testParameterMapState}", "the device is disconnected"), Eq(TestState::disconnected));
        EXPECT_THAT(Match<TestState>("the device is {testParameterMapState}", "the device is standby"), Eq(TestState::standby));
    }

    TEST(ParameterMap, cucumber_expression_does_not_match_unknown_keys)
    {
        EXPECT_THAT(Matches("the device is {testParameterMapState}", "the device is shutdown"), IsFalse());
        EXPECT_THAT(Matches("the device is {testParameterMapState}", "the device is connect"), IsFalse());
    }

    TEST(ParameterMap, cucumber_expression_is_case_sensitive)
    {
        EXPECT_THAT(Matches("the device is {testParameterMapState}", "the device is Connected"), IsFalse());
        EXPECT_THAT(Matches("the device is {testParameterMapState}", "the device is STANDBY"), IsFalse());
    }

    TEST(ParameterMap, cucumber_expression_resolves_prefix_and_special_character_keys)
    {
        EXPECT_THAT(Match<int>("number {testParameterMapNumber}", "number on"), Eq(100));
        EXPECT_THAT(Match<int>("number {testParameterMapNumber}", "number one"), Eq(1));
        EXPECT_THAT(Match<int>("number {testParameterMapNumber}", "number c++"), Eq(3));
        EXPECT_THAT(Match<int>("number {testParameterMapNumber}", "number a.b"), Eq(4));
        EXPECT_THAT(Matches("number {testParameterMapNumber}", "number axb"), IsFalse());
    }

    TEST(ParameterMap, supports_aggregate_values)
    {
        EXPECT_THAT(Match<TestVersion>("version {testParameterMapVersion}", "version v1"), Eq(TestVersion{ 1, 0 }));
        EXPECT_THAT(Match<TestVersion>("version {testParameterMapVersion}", "version v1.1"), Eq(TestVersion{ 1, 1 }));
    }

    TEST(ParameterMap, mapped_value_can_be_named_entries)
    {
        EXPECT_THAT(Match<int>("{testParameterMapEntriesName}", "seven"), Eq(7));
    }

    TEST(ParameterMap, entries_reject_duplicate_and_empty_keys)
    {
        using Entries = detail::ParameterMapEntries<int>;

        EXPECT_THROW((Entries{ { "a", 1 }, { "a", 2 } }), std::invalid_argument);
        EXPECT_THROW((Entries{ { "", 1 } }), std::invalid_argument);
        EXPECT_THROW((Entries{}), std::invalid_argument);
    }
}
