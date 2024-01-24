#ifndef CUCUMBER_CPP_TAGSTOSET_HPP
#define CUCUMBER_CPP_TAGSTOSET_HPP

#include <ranges>
#include <set>
#include <string>

namespace cucumber_cpp
{
    std::set<std::string> TagsToSet(const auto& tags)
    {
        std::set<std::string> result;

        for (const auto& entry : tags)
            result.insert(entry.name);

        return result;
    }
}

#endif
