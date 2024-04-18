#ifndef CUCUMBER_CPP_TAGSTOSET_HPP
#define CUCUMBER_CPP_TAGSTOSET_HPP

#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    auto TagsToSet(const auto& tags)
    {
        std::set<std::string, std::less<>> result;

        for (const auto& entry : tags)
            result.insert(entry.name);

        return result;
    }
}

#endif
