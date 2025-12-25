#include "cucumber_cpp/library/formatter/helper/KeywordType.hpp"
#include "cucumber/gherkin/dialect.hpp"
#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        constexpr std::array<std::string_view, 5> stepKeywords{
            "given",
            "when",
            "then",
            "and",
            "but",
        };
    }

    KeywordType GetStepKeywordType(std::string_view keyword, std::string_view language, std::optional<KeywordType> previousKeywordType)
    {
        const auto& dialect = cucumber::gherkin::keywords(language);
        const auto typeIter = std::ranges::find_if(stepKeywords, [&dialect, keyword](std::string_view stepKeyword)
            {
                return std::ranges::find(dialect.at(stepKeyword), keyword) != dialect.at(stepKeyword).end();
            });

        switch (std::distance(stepKeywords.begin(), typeIter))
        {
            case 0: // given
                return KeywordType::precondition;
            case 1: // when
                return KeywordType::event;
            case 2: // then
                return KeywordType::outcome;
            case 3: // and
            case 4: // but
                if (previousKeywordType.has_value())
                    return *previousKeywordType;
                [[fallthrough]];
            default:
                return KeywordType::precondition;
        }
    }
}
