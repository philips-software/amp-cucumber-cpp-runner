#ifndef FORMATTER_PRETTY_FORMATTER_HPP
#define FORMATTER_PRETTY_FORMATTER_HPP

#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include <cstddef>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <string>

namespace cucumber_cpp::library::formatter
{
    struct PrettyFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "pretty";

        struct Options
        {
            explicit Options(const nlohmann::json& formatOptions);

            const bool includeAttachments;
            const bool includeFeatureLine;
            const bool includeRuleLine;
            const bool useStatusIcon;
            const helper::Theme theme;
        };

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;

        void CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted);

        void HandleTestCaseStarted(const cucumber::messages::test_case_started& testCaseStarted);
        void HandleAttachment(const cucumber::messages::attachment& attachment);
        void HandleTestStepFinished(const cucumber::messages::test_step_finished& testStepFinished);
        void HandleTestRunFinished(const cucumber::messages::test_run_finished& testRunFinished);

        Options options{ formatOptions.contains(name) ? formatOptions.at(name) : nlohmann::json::object() };

        std::map<std::string, std::size_t> maxContentLengthByTestCaseStartedId;
        std::map<std::string, std::size_t> scenarioIndentByTestCaseStartedId;

        std::set<const cucumber::messages::feature*> printedFeatureUris;
        std::set<const cucumber::messages::rule*> printedRuleIds;
    };
}

#endif
