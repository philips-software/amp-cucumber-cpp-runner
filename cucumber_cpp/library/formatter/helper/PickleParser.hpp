#ifndef HELPER_PICKLE_PARSER_HPP
#define HELPER_PICKLE_PARSER_HPP

#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include <map>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    std::map<std::string, const cucumber::messages::pickle_step&> GetPickleStepMap(const cucumber::messages::pickle& pickle);

    std::string_view GetStepKeyword(const cucumber::messages::pickle_step& pickleStep, const GherkinStepMap& gherkinStepMap);
}

#endif
