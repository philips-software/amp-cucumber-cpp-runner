#ifndef FORMATTER_JUNIT_XML_FORMATTER_HPP
#define FORMATTER_JUNIT_XML_FORMATTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pugixml.hpp"
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter
{
    struct JunitXmlFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "junit";

    private:
        struct Options
        {
            explicit Options(const nlohmann::json& formatOptions);

            const std::string suiteName;
            const std::optional<std::string> testClassName;
        };

        void OnEnvelope(const cucumber::messages::envelope& envelope) override;

        Options options{ formatOptions.contains(name) ? formatOptions.at(name) : nlohmann::json::object() };

        pugi::xml_document doc;
        pugi::xml_node testSuite{ doc.append_child("testsuite") };
    };
}

#endif
