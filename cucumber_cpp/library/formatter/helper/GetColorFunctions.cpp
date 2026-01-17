
#include "cucumber_cpp/library/formatter/helper/GetColorFunctions.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "fmt/color.h"
#include "fmt/format.h"
#include <functional>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        template<fmt::color colour>
        std::string ColorStringFmt(std::string_view sv)
        {
            return fmt::format("{}", fmt::styled(sv, fmt::fg(colour)));
        }
    }

    std::function<std::string(std::string_view sv)> ColorFunctions::ForStatus(cucumber::messages::test_step_result_status status)
    {
        using enum cucumber::messages::test_step_result_status;

        switch (status)
        {
            case PASSED:
                return ColorStringFmt<fmt::color::green>;
            case SKIPPED:
                return ColorStringFmt<fmt::color::cyan>;
            case UNKNOWN:
            case PENDING:
            case UNDEFINED:
                return ColorStringFmt<fmt::color::yellow>;
            case AMBIGUOUS:
            case FAILED:
            default:
                return ColorStringFmt<fmt::color::red>;
        }
    }

    std::string ColorFunctions::Attachment(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::blue>(sv);
    }

    std::string ColorFunctions::Location(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::gray>(sv);
    }

    std::string ColorFunctions::Tag(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::cyan>(sv);
    }

    std::string ColorFunctions::DiffAdded(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::green>(sv);
    }

    std::string ColorFunctions::DiffRemoved(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::red>(sv);
    }

    std::string ColorFunctions::ErrorMessage(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::red>(sv);
    }

    std::string ColorFunctions::ErrorStack(std::string_view sv)
    {
        return ColorStringFmt<fmt::color::gray>(sv);
    }
}
