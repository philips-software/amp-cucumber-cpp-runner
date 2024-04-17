#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "args.hxx"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    struct GherkinParser
    {
        explicit GherkinParser(CucumberRunnerV2& cucumberRunner);

        [[nodiscard]] report::ReportHandler::Result RunFeatureFile(const std::filesystem::path& path);

    private:
        CucumberRunnerV2& cucumberRunner;
        std::unique_ptr<FeatureRunnerV2> featureRunner;

        cucumber::gherkin::app app;
        cucumber::gherkin::app::callbacks cbs;
    };

    struct Application
    {
        Application(std::span<const char*> args);

        Application(int argc, const char* const* argv);
        Application(int argc, const char* const* argv, args::Group& argsGroup);
        Application(int argc, const char* const* argv, std::vector<args::Group*> argsGroup);

        [[nodiscard]] const std::vector<std::string_view>& GetForwardArgs() const;

        void RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());

        [[nodiscard]] int GetExitCode() const;

        [[nodiscard]] report::Reporters& Reporters();

    private:
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;

        struct Options
        {
            explicit Options(std::span<const char*> args);

            std::vector<std::string_view> tags{};
            std::vector<std::string_view> features{};
            std::vector<std::string_view> reports{};
            std::vector<std::string_view> forwardArgs{};
        };

        std::optional<Options> options;

        /*
                    std::cout << "\n"
                              << name << " [--tag <tag>] --feature <file/folder> [--feature <file/folder>...] --report <reportname> [--report <reportname>...] [--Xapp,<forwardingargs>...]";
                    std::cout << "\n";
                    std::cout << "\n --tag <tag>              : cucumber tag expression";
                    std::cout << "\n --feature <file/folder>  : feature file or folder with feature files";
                    std::cout << "\n --report <reportname>    : name of the report generator";
                    std::cout << "\n                            junit-xml, console...";
                    std::cout << "\n --Xapp,<forwardingargs>  : arguments forwarded to application. Multiple arguments can be provided, separated by a comma";


        */

        args::ArgumentParser parser{ "", "" };
        args::HelpFlag argsHelp{ parser, "help", "Display this help menu", { "h", "help" } };
        args::NargsValueFlag<std::string> argsTags{ parser, "tags", "cucumber tag expression", { "t", "tag" }, { 0, std::numeric_limits<size_t>::max() } };
        args::NargsValueFlag<std::string> argsFeatures{ parser, "features", "feature file or folder with feature files", { "f", "feature" }, { 1, std::numeric_limits<size_t>::max() } };
        args::NargsValueFlag<std::string> argsReports{ parser, "reports", "name of the report generator", { "r", "report" }, { 1, std::numeric_limits<size_t>::max() } };

        report::ReportForwarder reporters;
        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };
    };
}

#endif
