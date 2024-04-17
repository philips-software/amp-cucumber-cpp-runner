#include "cucumber-cpp/Application.hpp"
#include "CLI/CLI.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber/gherkin/file.hpp"
#include "cucumber/gherkin/parse_error.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

namespace cucumber_cpp
{
    namespace
    {
        std::string_view const_char_to_sv(const char* value)
        {
            return { value };
        }

        std::string_view subrange_to_sv(const auto& subrange)
        {

            return { subrange.data(), subrange.data() + subrange.size() };
        }

        std::string JoinStringWithSpace(const std::string& a, std::string_view b)
        {
            return a + " " + std::string{ b };
        }

        std::filesystem::path to_fs_path(const std::string_view& sv)
        {
            return { sv };
        }

        bool is_feature_file(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }
    }

    GherkinParser::GherkinParser(CucumberRunnerV2& cucumberRunner)
        : cucumberRunner{ cucumberRunner }
        , callbacks{
            .ast = [&](const cucumber::gherkin::app::parser_result& ast)
            {
                featureRunner = cucumberRunner.StartFeature(ast);
            },
            .pickle = [&](const cucumber::messages::pickle& pickle)
            {
                featureRunner->StartScenario(pickle);
            },
            .error = [&](const cucumber::gherkin::parse_error& m) {}
        }
    {
        gherkin.include_source(false);
        gherkin.include_ast(true);
        gherkin.include_pickles(true);
    }

    report::ReportHandler::Result GherkinParser::RunFeatureFile(const std::filesystem::path& path)
    {
        gherkin.parse(cucumber::gherkin::file{ path.string() }, callbacks);
        auto result = featureRunner->Result();
        featureRunner = nullptr;
        return result;
    }

    Application::Application()
    {
        cli.add_option("-t,--tag", options.tags, "Cucumber tag expression")->expected(0, -1);
        cli.add_option("-f,--feature", options.features, "Feature file or folder with feature files")->required();
        cli.add_option("-r,--report", options.reporters, "Name of the report generator: ")->required();

        reporters.Add("console", std::make_unique<report::StdOutReportV2>());
        reporters.Add("junit-xml", std::make_unique<report::JunitReportV2>());
    }

    int Application::Run(int argc, const char* const* argv)
    {
        return Run(argc, argv, std::make_shared<ContextStorageFactoryImpl>());
    }

    int Application::Run(int argc, const char* const* argv, std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        try
        {
            const auto reportDescription = cli.get_option("--report")->get_description();
            const auto availableReporters = reporters.AvailableReporters();

            const auto joinedReporters = std::accumulate(availableReporters.begin() + 1, availableReporters.end(), reportDescription + availableReporters.front(), [](const std::string& lhs, const std::string& rhs)
                {
                    return lhs + ", " + rhs;
                });

            cli.get_option("--report")->description(joinedReporters);
            cli.parse(argc, argv);
            RunFeatures(std::move(contextStorageFactory));
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return cli;
    }

    void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandler>&& reporter)
    {
        reporters.Add(name, std::move(reporter));
    }

    void Application::RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        using Result = report::ReportHandler::Result;

        auto tagExpression = options.tags.empty() ? std::string{} : std::accumulate(std::next(options.tags.begin()), options.tags.end(), std::string(options.tags.front()), JoinStringWithSpace);

        CucumberRunnerV2 cucumberRunner{ {}, std::move(tagExpression), reporters, std::move(contextStorageFactory) };
        GherkinParser gherkinParser{ cucumberRunner };

        for (const auto& featurePath : GetFeatureFiles())
        {
            auto featureResult = gherkinParser.RunFeatureFile(featurePath);

            if (result == Result::undefined || result == Result::success)
                result = featureResult;
        }

        if (result == Result::undefined)
            std::cout << "\nError: no features have been executed";
    }

    int Application::GetExitCode() const
    {
        if (result == decltype(result)::success)
            return 0;
        else
            return 1;
    }

    std::vector<std::filesystem::path> Application::GetFeatureFiles() const
    {
        std::vector<std::filesystem::path> files;

        for (const auto feature : options.features | std::views::transform(to_fs_path))
            if (std::filesystem::is_directory(feature))
            {
                for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(is_feature_file))
                    files.push_back(entry.path());
            }
            else
                files.emplace_back(feature);

        return files;
    }
}
