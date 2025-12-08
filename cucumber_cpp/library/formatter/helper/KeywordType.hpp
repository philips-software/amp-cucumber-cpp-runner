#ifndef HELPER_KEYWORD_TYPE_HPP
#define HELPER_KEYWORD_TYPE_HPP

#include <optional>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    enum class KeywordType
    {
        precondition,
        event,
        outcome
    };

    KeywordType GetStepKeywordType(std::string_view keyword, std::string_view language, std::optional<KeywordType> previousKeywordType);
}

#endif
