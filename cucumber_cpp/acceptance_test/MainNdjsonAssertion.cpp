#include "util/NdjsonComparer.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <string>

int main(const int argc, char**argv)
{
    CLI::App app("Cucumber Ndjson reporter acceptance test assertion");

    std::string pathExpected;
    std::string pathActual;
    app.add_option("-e,--expected", pathExpected, "Path to expected ndjson report to compare against")->required();
    app.add_option("-a,--actual", pathActual, "Path to ndjson report to be validated")->required();

    CLI11_PARSE(app, argc, argv);

    NdjsonComparer comparer;
    const bool areEquivalent = comparer.AreEquivalent(pathExpected, pathActual);

    std::cout << (areEquivalent ? "true" : "false");

}
