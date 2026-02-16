#include "cucumber_cpp/library/Application.hpp"
#include "cucumber/gherkin/demangle.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
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
#include <fstream>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library
{
    namespace
    {
        bool IsFeatureFile(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }

        void CollectFilesFromDirectory(std::set<std::filesystem::path, std::less<>>& foundFiles, const std::filesystem::path& folder, const Application::Options& options)
        {
            if (options.recursive)
                for (const auto& entry : std::filesystem::recursive_directory_iterator{ folder } | std::views::filter(IsFeatureFile))
                    foundFiles.emplace(entry.path());
            else
                for (const auto& entry : std::filesystem::directory_iterator{ folder } | std::views::filter(IsFeatureFile))
                    foundFiles.emplace(entry.path());
        }

        std::set<std::filesystem::path, std::less<>> GetFeatureFiles(Application::Options& options)
        {
            std::set<std::filesystem::path, std::less<>> foundFiles;

            for (const auto& feature : options.paths)
                if (std::filesystem::is_directory(feature))
                    CollectFilesFromDirectory(foundFiles, feature, options);
                else if (IsFeatureFile(std::filesystem::directory_entry{ feature }))
                    foundFiles.emplace(feature);

            return foundFiles;
        }
    }

    Application::Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory, bool removeDefaultGoogleTestListener)
        : contextStorageFactory{ contextStorageFactory }
        , removeDefaultGoogleTestListener{ removeDefaultGoogleTestListener }
    {
        cli.set_config("--config", "cucumber.toml");
    }

    int Application::Run(int argc, const char* const* argv)
    {
        const auto formattersSet = formatters.GetAvailableFormatterNames();

        const auto formatterDescription = fmt::format("{{{}}}", fmt::join(formattersSet | std::views::transform([](const auto& pair)
                                                                                              {
                                                                                                  if (pair.second)
                                                                                                      return fmt::format("{}<<:output>>", pair.first);
                                                                                                  else
                                                                                                      return pair.first;
                                                                                              }),
                                                                    ","));

        CLI::Validator formatValidator{ [&formattersSet](const std::string& str) -> std::string
            {
                const api::FormatterOption option{ str };
                const auto iter = std::ranges::find(formattersSet, option.name, &std::pair<std::string, bool>::first);

                if (iter == formattersSet.end())
                    return fmt::format("'{}' is not a valid formatter", option.name);
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

            cli.add_flag("--dump-config", options.dumpConfig, "Dump the configuration");

            cli.add_flag("-d,--dry-run", options.dryRun, "Perform a dry run without executing steps")->default_val(options.dryRun);
            cli.add_flag("--fail-fast", options.failFast, "Stop execution on first failure")->default_val(options.failFast);
            cli.add_flag("--fail-global-hook-fast", options.failGlobalHookFast, "Stop execution on first global hook failure")->default_val(options.failGlobalHookFast);
            cli.add_option("--format", options.format, "specify the output format, optionally supply PATH to redirect formatter output.")->check(formatValidator)->default_val(options.format);
            cli.add_option("--format-options", options.formatOptions, "provide options for formatters")->default_val(options.formatOptions);
            cli.add_option("--language", options.language, "Default language for feature files, eg 'en'")->default_str(options.language);
            cli.add_option("--order", options.ordering, "Run scenarios in specificed order")->transform(CLI::CheckedTransformer(orderingMap, CLI::ignore_case))->default_val(options.ordering);
            auto* retryOpt = cli.add_option("--retry", options.retry, "Number of times to retry failed scenarios")->default_val(options.retry);
            cli.add_option("--retry-tag-filter", options.retryTagFilter, "Only retry scenarios matching this tag expression")->needs(retryOpt);
            cli.add_flag("--strict,!--no-strict", options.strict, "Fail if there are pending steps")->default_val(options.strict);
            cli.add_flag("--feature-hooks,!--no-feature-hooks", options.featureHooks, "Run Before/After Feature hooks, note these are non-standard and are not supported by messages")->default_val(options.featureHooks);
            cli.add_flag("--recursive,!--no-recursive", options.recursive, "Search for feature files recursively")->default_val(options.recursive);

            CLI::deprecate_option(cli.add_option("--tag", options.tags, "Cucumber tag expression"), "-t,--tags");
            cli.add_option("-t,--tags", options.tags, "Cucumber tag expression");

            CLI::deprecate_option(cli.add_option("-f,--feature", options.paths, "Paths to where your feature files are"), "paths");
            cli.add_option("paths", options.paths, "Paths to where your feature files are, defaults to \"./features\"")->default_val(options.paths);

            ProgramContext().InsertRef(options);

            cli.parse(argc, argv);

            if (options.dumpConfig)
                std::ofstream{ "cucumber.toml" } << cli.config_to_str(true, true);

            return RunFeatures();
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }
        catch (const InternalError& error)
        {
            std::cout << fmt::format("InternalError error:\n{}\n", error.what());
            return EXIT_FAILURE;
        }
        catch (const cucumber_expression::Error& error)
        {
            std::cout << fmt::format("Cucumber Expression error:\n{}\n", error.what());
            return EXIT_FAILURE;
        }
        catch (const std::exception& error)
        {
            std::cout << fmt::format("{}:\n{}\n", cucumber::gherkin::detail::demangle(typeid(error).name()).get(), error.what());
            return EXIT_FAILURE;
        }
        catch (...)
        {
            std::cout << "Unknown error";
            return EXIT_FAILURE;
        }
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

    int Application::RunFeatures()
    {
        fmt::println("Running with tags: {}", options.tags);

        const auto runOptions = support::RunOptions{
            .sources = {
                .paths = GetFeatureFiles(options),
                .tagExpression = tag_expression::Parse(fmt::to_string(fmt::join(options.tags, " "))),
                .ordering = options.ordering,
            },
            .runtime = {
                .dryRun = options.dryRun,
                .failFast = options.failFast,
                .failGlobalHookFast = options.failGlobalHookFast,
                .retry = options.retry,
                .strict = options.strict,
                .retryTagExpression = tag_expression::Parse(fmt::to_string(fmt::join(options.retryTagFilter, " "))),
                .featureHooks = options.featureHooks,
            },
        };

        const auto runPassed = api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster, formatters, options.format, options.formatOptions);
        return runPassed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
