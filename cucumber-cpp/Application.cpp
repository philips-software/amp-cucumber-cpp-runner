#include "cucumber-cpp/Application.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/report/JsonReport.hpp"
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber/gherkin/file.hpp"
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

        void ExitWithHelp(std::string_view name)
        {
            std::cout << "\n"
                      << name << " [--tag <tag>] --feature <file/folder> [--feature <file/folder>...] --report <reportname> [--report <reportname>...] [--Xapp,<forwardingargs>...]";
            std::cout << "\n";
            std::cout << "\n --tag <tag>              : cucumber tag expression";
            std::cout << "\n --feature <file/folder>  : feature file or folder with feature files";
            std::cout << "\n --report <reportname>    : name of the report generator";
            std::cout << "\n                            junit-xml, console...";
            std::cout << "\n --Xapp,<forwardingargs>  : arguments forwarded to application. Multiple arguments can be provided, separated by a comma";

            std::exit(1);
        }
    }

    Application::Options::Options(std::span<const char*> args)
    {
        const auto name = std::filesystem::path(const_char_to_sv(args[0])).filename().string();
        const auto view = args | std::views::drop(1) | std::views::transform(const_char_to_sv);

        for (auto current = view.begin(); current != view.end(); ++current)
        {
            const auto arg = *current;
            if (arg == "--tag")
            {
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    tags.push_back(*current);
                }
            }
            else if (arg == "--feature")
            {
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    features.push_back(*current);
                }
            }
            else if (arg == "--report")
            {
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    reports.push_back(*current);
                }
            }
            else if (arg.starts_with("--Xapp,"))
            {
                const auto param = arg.substr(std::string_view("--Xapp,").size());

                for (const auto xArg : std::views::split(param, ','))
                {
                    forwardArgs.push_back(subrange_to_sv(xArg));
                }
            }
            else
            {
                if (!(arg == "--help" && arg == "-h"))
                {
                    std::cout << "\nUnkown argument: " << std::quoted(arg) << "\n";
                }

                ExitWithHelp(name);
            }
        }

        const auto valid = [] {};
        const auto invalid = [&name]
        {
            ExitWithHelp(name);
        };

        const auto validateReports = [&, this](auto validate)
        {
            if (reports.empty())
            {
                std::cout << "\nno report generators";
                invalid();
            }
            else
            {
                validate();
            }
        };

        const auto validateArguments = [&, this]()
        {
            if (features.empty())
            {
                std::cout << "\nno feature files or folders";
                validateReports(invalid);
            }
            else
            {
                validateReports(valid);
            }
        };

        validateArguments();
    }

    Application::Application(std::span<const char*> args)
        : options{ args }
        , cbs{
            .ast = [&](const cucumber::gherkin::app::parser_result& ast)
            {
                nlohmann::json astJson;

                ast.to_json(astJson);

                root["features"].push_back({ { "ast", astJson } });
            },
            .pickle = [&](const cucumber::messages::pickle& pickle)
            {
                nlohmann::json scenarioJson;

                pickle.to_json(scenarioJson);

                root["features"].back()["scenarios"].push_back(scenarioJson);
            },
            .error = [&](const auto& m)
            {
                std::cout << m.to_json() << std::endl;
            }
        }
    {
        root["tagexpression"] = options.tags;
        app.include_source(false);
        app.include_ast(true);
        app.include_pickles(true);
    }

    const std::vector<std::string_view>& Application::GetForwardArgs() const
    {
        return options.forwardArgs;
    }

    void Application::RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        for (const auto& featurePath : GetFeatureFiles())
            app.parse(cucumber::gherkin::file{ featurePath }, cbs);

        const auto tagExpression = options.tags.empty() ? std::string{} : std::accumulate(std::next(options.tags.begin()), options.tags.end(), std::string(options.tags.front()), JoinStringWithSpace);

        CucumberRunner cucumberRunner{ GetForwardArgs(), tagExpression, std::move(contextStorageFactory) };
        cucumberRunner.Run(root);

        if (!root.contains("result"))
        {
            std::cout << "\nError: no features have been executed";
        }
    }

    void Application::GenerateReports(const std::map<std::string_view, report::Report&>& /*unused*/)
    {
        if (std::ranges::find(options.reports, "json") != options.reports.end())
        {
            cucumber_cpp::report::JsonReport report;
            report.GenerateReport(root);
        }

        if (std::ranges::find(options.reports, "console") != options.reports.end())
        {
            cucumber_cpp::report::StdOutReport report;
            report.GenerateReport(root);
        }

        if (std::ranges::find(options.reports, "junit-xml") != options.reports.end())
        {
            cucumber_cpp::report::JunitReport junitReport;
            junitReport.GenerateReport(root);
        }
    }

    int Application::GetExitCode() const
    {
        if (root.contains("result") && root["result"] == result::success)
        {
            return 0;
        }
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
