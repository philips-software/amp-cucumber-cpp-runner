#include "cucumber_cpp/library/cucumber_expression/TreeRegexp.hpp"
#include "cucumber/messages/group.hpp"
#include <cstddef>
#include <deque>
#include <list>
#include <optional>
#include <ranges>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        bool IsNonCapturing(std::string_view pattern, std::size_t pos)
        {
            if (pattern[pos + 1] != '?')
                return false;
            if (pattern[pos + 2] != '<')
                return true;

            return pattern[pos + 3] == '=' || pattern[pos + 3] == '!';
        }
    }

    void GroupBuilder::Add(GroupBuilder groupBuilder)
    {
        children.push_back(std::move(groupBuilder));
    }

    void GroupBuilder::SetNonCapturing()
    {
        capturing = false;
    }

    bool GroupBuilder::IsCapturing() const
    {
        return capturing;
    }

    void GroupBuilder::SetPattern(std::string_view pattern)
    {
        this->pattern = pattern;
    }

    void GroupBuilder::MoveChildrenTo(GroupBuilder& target)
    {
        for (auto& child : children)
            target.Add(std::move(child));
        children.clear();
    }

    const std::list<GroupBuilder>& GroupBuilder::Children() const
    {
        return children;
    }

    std::string_view GroupBuilder::Pattern() const
    {
        return pattern;
    }

    cucumber::messages::group GroupBuilder::Build(const std::smatch& match, std::size_t& index) const
    {
        const auto groupIndex = index++;
        const auto children = this->children | std::views::transform([&match, &index](const auto& child)
                                                   {
                                                       return child.Build(match, index);
                                                   });
        const auto value = match[groupIndex].matched ? std::make_optional(match[groupIndex].str()) : std::nullopt;

        return {
            .children = std::vector<cucumber::messages::group>(children.begin(), children.end()),
            .start = match[groupIndex].matched ? std::make_optional(match.position(groupIndex)) : std::nullopt,
            .value = value,
        };
    }

    /////////////////////////////////////////////////////////

    TreeRegexp::TreeRegexp(std::string_view pattern)
        : rootGroupBuilder{ CreateGroupBuilder(pattern) }
        , regex{ std::string(pattern) }
    {
    }

    const GroupBuilder& TreeRegexp::RootBuilder() const
    {
        return rootGroupBuilder;
    }

    std::optional<cucumber::messages::group> TreeRegexp::MatchToGroup(const std::string& text) const
    {
        std::smatch match;
        if (!std::regex_search(text, match, regex))
            return std::nullopt;

        std::size_t index = 0;
        return rootGroupBuilder.Build(match, index);
    }

    GroupBuilder TreeRegexp::CreateGroupBuilder(std::string_view pattern)
    {
        std::deque<GroupBuilder> stack;
        std::deque<std::size_t> groupStartStack;

        stack.emplace_back();

        bool escaping{ false };
        bool charClass{ false };

        for (std::size_t i = 0; i < pattern.size(); ++i)
        {
            const char c = pattern[i];

            if (c == '[' && !escaping)
                charClass = true;
            else if (c == ']' && !escaping)
                charClass = false;
            else if (c == '(' && !escaping && !charClass)
            {
                groupStartStack.emplace_back(i);
                auto& groupBuilder = stack.emplace_back();
                if (IsNonCapturing(pattern, i))
                    groupBuilder.SetNonCapturing();
            }
            else if (c == ')' && !escaping && !charClass)
            {
                if (stack.empty())
                    throw std::runtime_error("Empty stack");

                auto groupBuilder = stack.back();
                stack.pop_back();

                auto groupStart = groupStartStack.empty() ? 0 : groupStartStack.back();
                groupStart += 1;

                if (!groupStartStack.empty())
                    groupStartStack.pop_back();

                if (groupBuilder.IsCapturing())
                {
                    groupBuilder.SetPattern(pattern.substr(groupStart, i - groupStart));
                    stack.back().Add(groupBuilder);
                }
                else
                {
                    groupBuilder.MoveChildrenTo(stack.back());
                }
            }

            escaping = c == '\\' && !escaping;
        }

        if (stack.empty())
            throw std::runtime_error("Empty stack");

        return stack.back();
    }
}
