#ifndef HELPER_FORMAT_MESSAGES_HPP
#define HELPER_FORMAT_MESSAGES_HPP

#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include <cstddef>
#include <list>
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string FormatPickleTitle(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, const Theme& theme);
    std::string FormatPickleAttemptTitle(const cucumber::messages::pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::scenario& scenario, const Theme& theme);
    std::string FormatPickleLocation(const cucumber::messages::pickle& pickle, const std::optional<cucumber::messages::location>& location, const Theme& theme);
    std::string FormatStepText(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, cucumber::messages::test_step_result_status status, const Theme& theme);
    std::string FormatCodeLocation(const cucumber::messages::source_reference& sourceReference, const Theme& theme);
    std::string FormatCodeLocation(const cucumber::messages::step_definition* stepDefinition, const Theme& theme);
    std::string FormatFeatureTitle(const cucumber::messages::feature& feature, const Theme& theme);
    std::string FormatRuleTitle(const cucumber::messages::rule& rule, const Theme& theme);
    std::string FormatPickleTags(const cucumber::messages::pickle& pickle, const Theme& theme);
    std::string FormatHookTitle(const cucumber::messages::hook& hook, cucumber::messages::test_step_result_status status, bool isBeforeHook, bool useStatusIcon, const Theme& theme);
    std::string FormatStepTitle(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, cucumber::messages::test_step_result_status status, bool useStatusIcon, const Theme& theme);
    std::string FormatDocString(const cucumber::messages::pickle_doc_string& pickleDocString, const Theme& theme);
    std::string FormatDataTable(const cucumber::messages::pickle_table& pickleDataTable, const Theme& theme);
    std::string FormatPickleStepArgument(const cucumber::messages::pickle_step& pickleStep, const Theme& theme);
    std::string FormatAmbiguousStep(const std::list<const cucumber::messages::step_definition*>& stepDefinitions, const Theme& theme);
    std::string FormatTestStepResultError(const cucumber::messages::test_step_result& testStepResult, const Theme& theme);
    std::string FormatTestRunFinishedError(const cucumber::messages::test_run_finished& testRunFinished, const Theme& theme);
    std::string FormatBase64Attachment(const std::string& body, const std::string& mediaType, const std::optional<std::string>& filename, const Theme& theme);
    std::string FormatTextAttachment(const std::string& body, const Theme& theme);
    std::string FormatAttachment(const cucumber::messages::attachment& attachment, const Theme& theme);
}

#endif
