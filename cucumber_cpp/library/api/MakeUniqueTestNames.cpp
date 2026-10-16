#include "cucumber_cpp/library/api/MakeUniqueTestNames.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/GetHierarchicalTestName.hpp"
#include "fmt/core.h"
#include "fmt/format.h"
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>

namespace cucumber_cpp::library::api
{
    namespace
    {
        std::string MakeUnique(const std::string& testName, std::set<std::string, std::less<>>& seenNames)
        {
            auto uniqueName = testName;
            for (std::size_t counter = 2; seenNames.contains(uniqueName); ++counter)
                uniqueName = fmt::format("{}.#{}", testName, counter);

            seenNames.insert(uniqueName);
            return uniqueName;
        }
    }

    std::map<std::string, std::string, std::less<>> MakeUniqueTestNames(const cucumber::query::Query& query, const std::list<support::PickleSource>& pickleSources)
    {
        std::map<std::string, std::string, std::less<>> uniqueNames;
        std::set<std::string, std::less<>> seenNames;

        for (const auto& pickleSource : pickleSources)
            uniqueNames.emplace(pickleSource.pickle->id, MakeUnique(util::GetHierarchicalTestName(query, *pickleSource.pickle), seenNames));

        return uniqueNames;
    }
}
