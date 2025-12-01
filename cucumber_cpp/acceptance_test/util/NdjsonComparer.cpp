#include "NdjsonComparer.h"
#include <format>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool NdjsonComparer::AreEquivalent(const std::string& pathExpected, const std::string& pathActual)
{
    std::ifstream expected(pathExpected);
    std::string jsonLine;
    std::vector<json> expectedJsonObjects;
    while (std::getline(expected, jsonLine))
    {
        expectedJsonObjects.push_back(json::parse(jsonLine));
    }

    std::ifstream actual(pathActual);
    std::vector<json> actualJsonObjects;
    int lineNumber = 1;
    bool hasEmptyLine = false;
    while (std::getline(actual, jsonLine))
    {
        if (hasEmptyLine)
        {
            std::cerr << std::format("\nFailure: output contains unexpected empty line number {}.", lineNumber);
            return false;
        }
        if (jsonLine.empty())
        {
            hasEmptyLine = true;
            continue;
        }

        try
        {
            actualJsonObjects.push_back(json::parse(jsonLine));
        } catch (json::exception& e)
        {
            std::cerr << std::format("\nFailure: line {} contains invalid json. {}", lineNumber, e.what());
        }
        lineNumber++;
    }

    for (int i = 0; i < expectedJsonObjects.size(); i++)
    {
        if (i > actualJsonObjects.size() - 1)
        {
            std::cerr << std::format("\nFailure: Unexpectedly reached the end of generated output. Expected {} but received {} objects.", expectedJsonObjects.size(), actualJsonObjects.size());
            return false;
        }
        // Missing of the keys at the first level implies that the entire event is missing
        json::iterator topLevelKeyIterator = expectedJsonObjects[i].begin();
        while (topLevelKeyIterator != expectedJsonObjects[i].end())
        {
            const json& actualJson = actualJsonObjects[i];
            const std::string& expectedKey = topLevelKeyIterator.key();
            if (actualJson.contains(expectedKey))
            {
                // TODO Perform event data assertions. Dynamic values, such as timestamps and paths, will need special handling
                ++topLevelKeyIterator;
            } else {
                std::cerr << std::format("\nFailure: Event \"{}\" is missing on line {}. Found \"{}\" instead.", expectedKey, i, actualJson.begin().key());

                return false;
            }
        }
    }

    return true;
}
