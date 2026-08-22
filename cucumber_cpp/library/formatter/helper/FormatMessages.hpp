#ifndef HELPER_FORMAT_MESSAGES_HPP
#define HELPER_FORMAT_MESSAGES_HPP

#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/PickleTable.hpp"
#include "cucumber/messages/Rule.hpp"
#include "cucumber/messages/Scenario.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber/messages/Step.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include <cstddef>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    std::string FormatPickleTitle(const cucumber::messages::Pickle& pickle, const cucumber::messages::Scenario& scenario, const Theme& theme);
    std::string FormatPickleAttemptTitle(const cucumber::messages::Pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::Scenario& scenario, const Theme& theme);
    std::string FormatPickleLocation(const cucumber::messages::Pickle& pickle, const std::shared_ptr<const cucumber::messages::Location>& location, const Theme& theme);
    std::string FormatStepText(const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, cucumber::messages::TestStepResultStatus status, const Theme& theme);
    std::string FormatCodeLocation(const cucumber::messages::SourceReference& sourceReference, const Theme& theme);
    std::string FormatCodeLocation(const cucumber::messages::StepDefinition* stepDefinition, const Theme& theme);
    std::string FormatFeatureTitle(const cucumber::messages::Feature& feature, const Theme& theme);
    std::string FormatRuleTitle(const cucumber::messages::Rule& rule, const Theme& theme);
    std::string FormatPickleTags(const cucumber::messages::Pickle& pickle, const Theme& theme);
    std::string FormatHookTitle(const cucumber::messages::Hook& hook, cucumber::messages::TestStepResultStatus status, bool isBeforeHook, bool useStatusIcon, const Theme& theme);
    std::string FormatStepTitle(const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::Step& step, cucumber::messages::TestStepResultStatus status, bool useStatusIcon, const Theme& theme);
    std::string FormatDocString(const cucumber::messages::PickleDocString& pickleDocString, const Theme& theme);
    std::string FormatDataTable(const cucumber::messages::PickleTable& pickleDataTable, const Theme& theme);
    std::string FormatPickleStepArgument(const cucumber::messages::PickleStep& pickleStep, const Theme& theme);
    std::string FormatAmbiguousStep(const std::list<const cucumber::messages::StepDefinition*>& stepDefinitions, const Theme& theme);
    std::string FormatTestStepResultError(const cucumber::messages::TestStepResult& testStepResult, const Theme& theme);
    std::string FormatTestRunFinishedError(const cucumber::messages::TestRunFinished& testRunFinished, const Theme& theme);
    std::string FormatBase64Attachment(std::string_view body, const std::string& mediaType, const std::optional<std::string>& filename, const Theme& theme);
    std::string FormatTextAttachment(const std::string& body, const Theme& theme);
    std::string FormatAttachment(const cucumber::messages::Attachment& attachment, const Theme& theme);
}

#endif
