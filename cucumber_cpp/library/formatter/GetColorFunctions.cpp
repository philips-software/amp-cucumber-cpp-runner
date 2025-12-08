
#include "cucumber_cpp/library/formatter/GetColorFunctions.hpp"
#include "cpp-terminal/color.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "rang.hpp"
#include <format>
#include <functional>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        template<Term::Color::Name colour>
        std::string ColorString(std::string_view sv)
        {
            return std::format("{}{}{}", Term::color_fg(colour), sv, Term::color_fg(Term::Color::Name::Default));
        }
    }

    std::function<std::string(std::string_view sv)>
    ColorFunctions::ForStatus(cucumber::messages::test_step_result_status status)
    {
        using enum cucumber::messages::test_step_result_status;

        switch (status)
        {
            case PASSED:
                return ColorString<Term::Color::Name::Green>;
            case SKIPPED:
                return ColorString<Term::Color::Name::Cyan>;
            case UNKNOWN:
            case PENDING:
            case UNDEFINED:
                return ColorString<Term::Color::Name::Yellow>;
            case AMBIGUOUS:
            case FAILED:
            default:
                return ColorString<Term::Color::Name::Red>;
        }
    }

    std::string ColorFunctions::Location(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Gray>(sv);
    }

    std::string ColorFunctions::Tag(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Cyan>(sv);
    }

    std::string ColorFunctions::DiffAdded(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Green>(sv);
    }

    std::string ColorFunctions::DiffRemoved(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Red>(sv);
    }

    std::string ColorFunctions::ErrorMessage(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Red>(sv);
    }

    std::string ColorFunctions::ErrorStack(std::string_view sv)
    {
        return ColorString<Term::Color::Name::Gray>(sv);
    }

    ColorFunctions GetColorFunctions(bool useColors)
    {
        std::string str;

        str += Term::color_fg(Term::Color::Name::Red);
        str += "test";

        // str += rang::fg::red;

        return {};
    }
}
