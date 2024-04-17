#include "cucumber-cpp/Application.hpp"
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

        [[noreturn]] void ExitWithHelp(std::string_view name)
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
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    tags.push_back(*current);
                }
            else if (arg == "--feature")
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    features.push_back(*current);
                }
            else if (arg == "--report")
                while (std::next(current) != view.end() && (!(*std::next(current)).starts_with("-")))
                {
                    current = std::next(current);
                    reports.push_back(*current);
                }
            else if (arg.starts_with("--Xapp,"))
            {
                const auto param = arg.substr(std::string_view("--Xapp,").size());

                for (const auto xArg : std::views::split(param, ','))
                    forwardArgs.push_back(subrange_to_sv(xArg));
            }
            else
            {
                if (!(arg == "--help" && arg == "-h"))
                    std::cout << "\nUnkown argument: " << std::quoted(arg) << "\n";

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

    GherkinParser::GherkinParser(CucumberRunnerV2& cucumberRunner)
        : cucumberRunner{ cucumberRunner }
        , cbs{
            .ast = [this](const cucumber::gherkin::app::parser_result& ast)
            {
                featureRunner = this->cucumberRunner.StartFeature(ast);
            },
            .pickle = [this](const cucumber::messages::pickle& pickle)
            {
                featureRunner->StartScenario(pickle);
            },
            .error = [](const cucumber::gherkin::parse_error& /* _ */)
            {
                /* not handled yet */
            }
        }
    {
        app.include_source(false);
        app.include_ast(true);
        app.include_pickles(true);
    }

    report::ReportHandler::Result GherkinParser::RunFeatureFile(const std::filesystem::path& path)
    {
        app.parse(cucumber::gherkin::file{ path.string() }, cbs);
        auto result = featureRunner->Result();
        featureRunner = nullptr;
        return result;
    }

    ResultStatus& ResultStatus::operator=(Result result)
    {
        if ((resultStatus == Result::undefined || resultStatus == Result::success) && result != Result::undefined)
            resultStatus = result;

        return *this;
    }

    ResultStatus::operator Result() const
    {
        return resultStatus;
    }

    bool ResultStatus::IsSuccess() const
    {
        return resultStatus == Result::success;
    }

    Application::Application(std::span<const char*> args)
        : options{ args }
    {
        if (std::ranges::find(options.reports, "console") != options.reports.end())
            reporters.Add(std::make_unique<report::StdOutReportV2>());

        if (std::ranges::find(options.reports, "junit-xml") != options.reports.end())
            reporters.Add(std::make_unique<report::JunitReportV2>());
    }

    const std::vector<std::string_view>& Application::GetForwardArgs() const
    {
        return options.forwardArgs;
    }

    void Application::RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        auto tagExpression = options.tags.empty() ? std::string{} : std::accumulate(std::next(options.tags.begin()), options.tags.end(), std::string(options.tags.front()), JoinStringWithSpace);

        CucumberRunnerV2 cucumberRunner{ GetForwardArgs(), std::move(tagExpression), reporters, std::move(contextStorageFactory) };
        GherkinParser gherkinParser{ cucumberRunner };

        for (const auto& featurePath : GetFeatureFiles())
        {
            resultStatus = gherkinParser.RunFeatureFile(featurePath);
        }

        if (static_cast<ResultStatus::Result>(resultStatus) == ResultStatus::Result::undefined)
            std::cout << "\nError: no features have been executed";
    }

    int Application::GetExitCode() const
    {
        if (resultStatus.IsSuccess())
            return 0;
        else
            return 1;
    }

    [[nodiscard]] report::Reporters& Application::Reporters()
    {
        return reporters;
    }

    std::vector<std::filesystem::path> Application::GetFeatureFiles() const
    {
        std::vector<std::filesystem::path> files;

        for (const auto feature : options.features | std::views::transform(to_fs_path))
            if (std::filesystem::is_directory(feature))
                for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(is_feature_file))
                    files.push_back(entry.path());
            else
                files.emplace_back(feature);

        return files;
    }
}
