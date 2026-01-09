#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include <CLI/App.hpp>
#include <CLI/Error.hpp>
#include <CLI/Option.hpp>
#include <CLI/Validators.hpp>
#include <CLI/impl/App_inl.hpp>
#include <CLI/impl/Option_inl.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace cucumber_cpp::library
{
    namespace
    {
        template<class Range>
        std::string Join(const Range& range, std::string_view delim)
        {
            if (range.empty())
                return "";

            return std::accumulate(std::next(range.begin()), range.end(), range.front(), [&delim](const auto& lhs, const auto& rhs)
                {
                    return std::format("{}{}{}", lhs, delim, rhs);
                });
        }

        std::filesystem::path ToFileSystemPath(const std::string_view& sv)
        {
            return { sv };
        }

        bool IsFeatureFile(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }

        std::set<std::filesystem::path> GetFeatureFiles(Application::Options& options)
        {
            std::set<std::filesystem::path> files;

            for (const auto feature : options.paths | std::views::transform(ToFileSystemPath))
                if (std::filesystem::is_directory(feature))
                    for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(IsFeatureFile))
                        files.emplace(entry.path());
                else
                    files.emplace(feature);

            return files;
        }
    }

    Application::Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory, bool removeDefaultGoogleTestListener)
        : contextStorageFactory{ contextStorageFactory }
        , removeDefaultGoogleTestListener{ removeDefaultGoogleTestListener }
    {
        cli.parse_complete_callback([this]
            {
                RunFeatures();
            });
    }

    int Application::Run(int argc, const char* const* argv)
    {
        const auto formattersSet = formatters.GetAvailableFormatterNames();
        const auto formatterDescription = std::format("{{{}}}", Join(formattersSet | std::views::transform([](const auto& pair)
                                                                                         {
                                                                                             if (pair.second)
                                                                                                 return std::format("{}<<:output>>", pair.first);
                                                                                             else
                                                                                                 return pair.first;
                                                                                         }),
                                                                    ","));

        CLI::Validator formatValidator{ [&formattersSet](const std::string& str) -> std::string
            {
                const api::FormatterOption option{ str };
                const auto iter = std::ranges::find(formattersSet, option.name, &std::pair<std::string, bool>::first);

                if (iter == formattersSet.end())
                    return std::format("'{}' is not a valid formatter", option.name);
                else
                    return "";
            },
            formatterDescription };

        try
        {
            const std::map<std::string, support::RunOptions::Ordering, std::less<>> orderingMap{
                { "defined", support::RunOptions::Ordering::defined },
                { "reverse", support::RunOptions::Ordering::reverse },
            };

            cli.add_flag("-d,--dry-run", options.dryRun, "Perform a dry run without executing steps");
            cli.add_flag("--fail-fast", options.failFast, "Stop execution on first failure");
            cli.add_option("--format", options.format, "specify the output format, optionally supply PATH to redirect formatter output.")->check(formatValidator);
            cli.add_option("--format-options", options.formatOptions, "provide options for formatters");
            cli.add_option("--language", options.language, "Default langauge for feature files, eg 'en'")->default_str(options.language);
            cli.add_option("--order", options.ordering, "Run scenarios in specificed order")->transform(CLI::CheckedTransformer(orderingMap, CLI::ignore_case));
            auto* retryOpt = cli.add_option("--retry", options.retry, "Number of times to retry failed scenarios")->default_val(options.retry);
            cli.add_option("--retry-tag-filter", options.retryTagFilter, "Only retry scenarios matching this tag expression")->needs(retryOpt);
            cli.add_flag("--strict,!--no-strict", options.strict, "Fail if there are pending steps")->default_val(options.strict);

            CLI::deprecate_option(cli.add_option("--tag", options.tags, "Cucumber tag expression"), "-t,--tags");
            cli.add_option("-t,--tags", options.tags, "Cucumber tag expression");

            CLI::deprecate_option(cli.add_option("-f,--feature", options.paths, "Paths to where your feature files are")->check(CLI::ExistingPath), "paths");
            cli.add_option("paths", options.paths, "Paths to where your feature files are")->required()->check(CLI::ExistingPath);

            ProgramContext().InsertRef(options);

            cli.parse(argc, argv);
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }
        catch (const InternalError& error)
        {
            std::cout << std::format("InternalError error:\n{}\n", error.what());
            return 1;
        }
        catch (const cucumber_expression::Error& error)
        {
            std::cout << std::format("Cucumber Expression error:\n{}\n", error.what());
            return 1;
        }
        catch (const std::exception& error)
        {
            std::cout << std::format("Generic error:\n{}\n", error.what());
            return 1;
        }
        catch (...)
        {
            std::cout << "Unknown error";
            return 1;
        }

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return cli;
    }

    Context& Application::ProgramContext()
    {
        return programContextRef;
    }

    cucumber_expression::ParameterRegistry& Application::ParameterRegistration()
    {
        return parameterRegistry;
    }

    api::Formatters& Application::Formatters()
    {
        return formatters;
    }

    void Application::RunFeatures()
    {
        const auto runOptions = support::RunOptions{
            .sources = {
                .paths = GetFeatureFiles(options),
                .tagExpression = tag_expression::Parse(Join(options.tags, " ")),
                .ordering = options.ordering,
            },
            .runtime = {
                .dryRun = options.dryRun,
                .failFast = options.failFast,
                .retry = options.retry,
                .strict = options.strict,
                .retryTagExpression = tag_expression::Parse(Join(options.retryTagFilter, " ")),
            },
        };

        runPassed = api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster, formatters, options.format, options.formatOptions);
    }

    int Application::GetExitCode() const
    {
        return runPassed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
