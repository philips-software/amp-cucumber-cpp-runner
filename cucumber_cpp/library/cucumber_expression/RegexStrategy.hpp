#ifndef CUCUMBER_EXPRESSION_REGEXSTRATEGY_HPP
#define CUCUMBER_EXPRESSION_REGEXSTRATEGY_HPP

#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace re2
{
    class RE2;
}

namespace cucumber_cpp::library::cucumber_expression
{
    struct RegexStrategy
    {
        RegexStrategy() = default;
        RegexStrategy(const RegexStrategy&) = delete;
        RegexStrategy& operator=(const RegexStrategy&) = delete;
        RegexStrategy(RegexStrategy&&) = delete;
        RegexStrategy& operator=(RegexStrategy&&) = delete;
        virtual ~RegexStrategy() = default;

        [[nodiscard]] virtual std::optional<std::vector<std::string>> Match(std::string_view text) const = 0;
    };

    struct StdRegexStrategy : RegexStrategy
    {
        explicit StdRegexStrategy(std::string_view pattern);

        [[nodiscard]] std::optional<std::vector<std::string>> Match(std::string_view text) const override;

    private:
        std::regex regex;
    };

    struct Re2RegexStrategy : RegexStrategy
    {
        explicit Re2RegexStrategy(std::string_view pattern);
        ~Re2RegexStrategy() override;

        [[nodiscard]] std::optional<std::vector<std::string>> Match(std::string_view text) const override;

    private:
        std::unique_ptr<re2::RE2> re2;
    };
}

#endif
