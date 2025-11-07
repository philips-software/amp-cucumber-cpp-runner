#include "NdjsonComparer.h"
#include <fstream>

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
    while (std::getline(actual, jsonLine))
    {
        actualJsonObjects.push_back(json::parse(jsonLine));
    }

    if (expectedJsonObjects.size() != actualJsonObjects.size())
    {
        return false;
    }

    for (size_t i = 0; i < expectedJsonObjects.size(); i++)
    {
        json expectedJson = expectedJsonObjects[i];
        json actualJson = actualJsonObjects[i];

        if (expectedJson.size() != actualJson.size())
        {
            return false;
        }

        const json::iterator expectedIterator = expectedJson.begin();
        const json::iterator actualIterator = actualJson.begin();

        while (expectedIterator != expectedJson.end())
        {
            // TODO this will most definitely fail without predictable sorting
            if (expectedIterator.key() != actualIterator.key() ||
                expectedIterator.value() != actualIterator.value())
            {
                // TODO dynamic values such as timestamps and paths will need special handling
                return false;
            }
        }
    }

    return true;
}