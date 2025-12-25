#ifndef FORMATTER_GET_COLOR_FUNCTIONS_HPP
#define FORMATTER_GET_COLOR_FUNCTIONS_HPP

#include "cucumber/messages/test_step_result_status.hpp"
#include <functional>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    struct ColorFunctions
    {
        static std::function<std::string(std::string_view)> ForStatus(cucumber::messages::test_step_result_status status);
        static std::string Location(std::string_view);
        static std::string Tag(std::string_view);
        static std::string DiffAdded(std::string_view);
        static std::string DiffRemoved(std::string_view);
        static std::string ErrorMessage(std::string_view);
        static std::string ErrorStack(std::string_view);
    };
}

#endif
