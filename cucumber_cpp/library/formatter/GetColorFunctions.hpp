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
        std::function<std::string(std::string_view)> ForStatus(cucumber::messages::test_step_result_status status);
        std::string Location(std::string_view);
        std::string Tag(std::string_view);
        std::string DiffAdded(std::string_view);
        std::string DiffRemoved(std::string_view);
        std::string ErrorMessage(std::string_view);
        std::string ErrorStack(std::string_view);
    };

    ColorFunctions GetColorFunctions(bool useColors);
}

#endif
