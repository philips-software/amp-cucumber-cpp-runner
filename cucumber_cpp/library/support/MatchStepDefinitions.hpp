#ifndef SUPPORT_MATCH_STEP_DEFINITIONS_HPP
#define SUPPORT_MATCH_STEP_DEFINITIONS_HPP

#include "cucumber/messages/TestStep.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include <list>
#include <string>

namespace cucumber_cpp::library::support
{
    // Populates testStep.stepDefinitionIds/stepMatchArgumentsLists (must already be set) with every definition matching text.
    void MatchStepDefinitions(cucumber::messages::TestStep& testStep, const std::list<StepRegistry::Definition>& stepDefinitions, const std::string& text);
}

#endif
