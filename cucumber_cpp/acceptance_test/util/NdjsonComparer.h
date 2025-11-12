#ifndef CUCUMBER_CPP_RUNNER_NDJSONCOMPARER_H
#define CUCUMBER_CPP_RUNNER_NDJSONCOMPARER_H

#include <nlohmann/json.hpp>

class NdjsonComparer
{
public:
    [[nodiscard]]
    static bool AreEquivalent(const std::string& pathExpected, const std::string& pathActual);
};

#endif // CUCUMBER_CPP_RUNNER_NDJSONCOMPARER_H
