#ifndef CUCUMBER_EXPRESSION_TREE_REGEXP_HPP
#define CUCUMBER_EXPRESSION_TREE_REGEXP_HPP

#include "cucumber_cpp/library/cucumber_expression/Group.hpp"
#include <cstddef>
#include <list>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::cucumber_expression
{
    struct GroupBuilder
    {
        void Add(GroupBuilder groupBuilder);

        void SetNonCapturing();
        [[nodiscard]] bool IsCapturing() const;

        void SetPattern(std::string_view pattern);

        void MoveChildrenTo(GroupBuilder& target);

        [[nodiscard]] const std::list<GroupBuilder>& Children() const;
        [[nodiscard]] std::string_view Pattern() const;

        [[nodiscard]] ArgumentGroup Build(const std::smatch& match, std::size_t& index) const;

    private:
        std::string_view pattern;
        bool capturing{ true };
        std::list<GroupBuilder> children;
    };

    struct TreeRegexp
    {
        explicit TreeRegexp(std::string_view pattern);

        [[nodiscard]] const GroupBuilder& RootBuilder() const;

        [[nodiscard]] std::optional<ArgumentGroup> MatchToGroup(const std::string& text) const;

    private:
        GroupBuilder rootGroupBuilder;
        std::regex regex;
    };
}

#endif
