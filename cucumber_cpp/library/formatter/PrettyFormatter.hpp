#ifndef FORMATTER_PRETTY_FORMATTER_HPP
#define FORMATTER_PRETTY_FORMATTER_HPP

#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/Rule.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <cstddef>
#include <map>
#include <memory>
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
        void OnEnvelope(const cucumber::messages::Envelope& envelope) override;

        void CalculateIndent(const std::shared_ptr<const cucumber::messages::TestCaseStarted>& testCaseStarted);

        void HandleTestCaseStarted(const std::shared_ptr<const cucumber::messages::TestCaseStarted>& testCaseStarted);
        void HandleAttachment(const cucumber::messages::Attachment& attachment);
        void HandleTestStepFinished(const std::shared_ptr<const cucumber::messages::TestStepFinished>& testStepFinished);
        void HandleTestRunFinished(const cucumber::messages::TestRunFinished& testRunFinished);

        Options options{ formatOptions.contains(name) ? formatOptions.at(name) : nlohmann::json::object() };

        std::map<std::string, std::size_t> maxContentLengthByTestCaseStartedId;
        std::map<std::string, std::size_t> scenarioIndentByTestCaseStartedId;

        std::set<const cucumber::messages::Feature*> printedFeatureUris;
        std::set<const cucumber::messages::Rule*> printedRuleIds;
    };
}

#endif
