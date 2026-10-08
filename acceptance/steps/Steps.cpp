#include "cucumber_cpp/Steps.hpp"
#include "acceptance/util/Subprocess.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cucumber/query/EnvelopeArchive.hpp>
#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
    using acceptance::util::ProcessResult;
    using acceptance::util::RunProcess;
    using cucumber_cpp::library::plugin::DynamicLibrary;

    constexpr std::string_view scenarioWorkspace = "scenario_workspace";
    constexpr std::string_view lastBuildResult = "last_build_result";
    constexpr std::string_view lastMessageOutputPath = "last_message_output_path";
    constexpr std::string_view lastPrettyOutputPath = "last_pretty_output_path";
    constexpr std::string_view lastSummaryOutputPath = "last_summary_output_path";

    class MessageFormatterOutputError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    std::uint64_t GetProcessId()
    {
#if defined(_WIN32)
        return static_cast<std::uint64_t>(GetCurrentProcessId());
#else
        return static_cast<std::uint64_t>(getpid());
#endif
    }

    [[nodiscard]] bool ContainsParentReference(const std::filesystem::path& path)
    {
        return std::any_of(path.begin(), path.end(), [](const auto& part)
            {
                return part == "..";
            });
    }

    [[nodiscard]] std::filesystem::path ValidateRelativePath(const std::string& name)
    {
        const std::filesystem::path path{ name };
        if (name.empty() || path.is_absolute() || ContainsParentReference(path))
            throw std::runtime_error{ "Path must be relative and must not contain '..'" };

        return path;
    }

    [[nodiscard]] std::string ValidateLibraryName(const std::string& name)
    {
        const auto libraryPath = ValidateRelativePath(name);
        if (libraryPath.has_parent_path())
            throw std::runtime_error{ "Library name must not contain path separators" };

        return name;
    }

    [[nodiscard]] std::string ReplaceSpacesWithUnderscores(std::string value)
    {
        for (auto& character : value)
            if (character == ' ')
                character = '_';

        return value;
    }

    [[nodiscard]] std::filesystem::path CreateWorkspacePath()
    {
        const auto now = static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        const auto processId = GetProcessId();

        for (std::uint64_t attempt = 0; attempt < 64; ++attempt)
        {
            const auto folderName =
                "ccr_acceptance_" + std::to_string(processId) + "_" + std::to_string(now) + "_" + std::to_string(attempt);
            const auto path = std::filesystem::temp_directory_path() / folderName;
            std::error_code error{};
            if (std::filesystem::create_directory(path, error))
                return path;
        }

        throw std::runtime_error{ "Failed to create acceptance workspace" };
    }

    [[nodiscard]] std::string FormatProcessResult(const ProcessResult& result)
    {
        std::ostringstream out;
        out << "Exit code: " << result.exitCode << "\n";
        out << "--- stdout ---\n"
            << result.standardOutput << "\n";
        out << "--- stderr ---\n"
            << result.standardError << "\n";
        return out.str();
    }

    [[nodiscard]] cucumber::query::Query LoadQueryFromMessageFile(const std::filesystem::path& messagePath, cucumber::query::EnvelopeArchive& envelopeArchive)
    {
        std::ifstream input(messagePath, std::ios::binary);
        if (!input.is_open())
            throw MessageFormatterOutputError{ "Could not read message formatter output: " + messagePath.string() };

        cucumber::query::Query query;
        std::string line;
        while (std::getline(input, line))
        {
            if (line.empty())
                continue;

            const auto json = nlohmann::json::parse(line);

            query.Update(envelopeArchive.Store(json.get<cucumber::messages::Envelope>()));
        }

        return query;
    }
}

HOOK_BEFORE_SCENARIO()
{
    context.InsertAt<std::filesystem::path>(scenarioWorkspace, CreateWorkspacePath());
}

HOOK_AFTER_SCENARIO()
{
    if (!context.Contains(scenarioWorkspace))
        return;

    std::error_code error{};
    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    std::filesystem::remove_all(workspace, error);
}

GIVEN(R"(a file named {string} with:)", (const std::string& filename))
{
    ASSERT_THAT(docString, testing::IsTrue());

    const auto sanitizedFilename = ReplaceSpacesWithUnderscores(filename);

    const auto relativePath = ValidateRelativePath(sanitizedFilename);
    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto filePath = workspace / relativePath;

    if (filePath.has_parent_path())
        std::filesystem::create_directories(filePath.parent_path());

    std::ofstream output(filePath, std::ios::binary);
    ASSERT_THAT(output.is_open(), testing::IsTrue()) << "Could not write feature file: " << filePath;

    output << docString->content;
}

GIVEN(R"(a library named {string} with:)", (const std::string& libraryName))
{
    ASSERT_THAT(docString, testing::IsTrue());

    const auto sanitizedLibraryName = ReplaceSpacesWithUnderscores(libraryName);
    const auto validatedName = ValidateLibraryName(sanitizedLibraryName);

    {
        std::ofstream sourceFile(CCR_ACCEPTANCE_SANDBOX_SOURCE, std::ios::binary);
        ASSERT_THAT(sourceFile.is_open(), testing::IsTrue()) << "Could not write sandbox source: " << CCR_ACCEPTANCE_SANDBOX_SOURCE;
        sourceFile << docString->content;
    }

    const auto buildResult = RunProcess(
        std::filesystem::path{ CCR_ACCEPTANCE_CMAKE_COMMAND },
        {
            "--build",
            CCR_ACCEPTANCE_BUILD_DIR,
            "--target",
            CCR_ACCEPTANCE_SANDBOX_TARGET,
        });

    context.InsertAt<ProcessResult>(lastBuildResult, buildResult);

    ASSERT_THAT(buildResult.exitCode, testing::Eq(0))
        << "Sandbox build failed\n"
        << FormatProcessResult(buildResult);

    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto extension = std::string{ DynamicLibrary::PlatformExtension() };
    const auto outputPath = workspace / (validatedName + extension);

    std::filesystem::copy_file(
        CCR_ACCEPTANCE_SANDBOX_ARTIFACT,
        outputPath,
        std::filesystem::copy_options::overwrite_existing);
}

WHEN(R"(I run cucumber-cpp-runner with {string})", (const std::string& libraryName))
{
    const auto validatedName = ValidateLibraryName(libraryName);
    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto extension = std::string{ DynamicLibrary::PlatformExtension() };
    const auto libraryPath = workspace / (validatedName + extension);
    const auto messageOutputPath = workspace / "messages.ndjson";
    const auto prettyOutputPath = workspace / "pretty.txt";
    const auto summaryOutputPath = workspace / "summary.txt";

    ASSERT_THAT(std::filesystem::is_regular_file(libraryPath), testing::IsTrue())
        << "Expected plugin library at: " << libraryPath;

    const auto runResult = RunProcess(
        std::filesystem::path{ CCR_ACCEPTANCE_CLI },
        {
            "--load",
            libraryPath.string(),
            "--format",
            "pretty:" + prettyOutputPath.string(),
            "message:" + messageOutputPath.string(),
            "summary:" + summaryOutputPath.string(),
            "--format-options",
            R"({ "pretty": {"theme" : "plain"}, "summary": {"theme" : "plain"} })",
            "--",
            workspace.string(),
        },
        workspace);

    context.Insert<ProcessResult>(runResult);
    context.Insert<cucumber::query::Query>(LoadQueryFromMessageFile(messageOutputPath, *context.Emplace<cucumber::query::EnvelopeArchive>()));

    context.InsertAt<std::filesystem::path>(lastMessageOutputPath, messageOutputPath);
    context.InsertAt<std::filesystem::path>(lastPrettyOutputPath, prettyOutputPath);
    context.InsertAt<std::filesystem::path>(lastSummaryOutputPath, summaryOutputPath);
}

THEN("it passes")
{
    const auto& processResult = context.Get<ProcessResult>();
    ASSERT_THAT(processResult.exitCode, testing::Eq(0)) << FormatProcessResult(processResult);
}

THEN("it fails")
{
    const auto& processResult = context.Get<ProcessResult>();
    ASSERT_THAT(processResult.exitCode, testing::Ne(0)) << FormatProcessResult(processResult);
}
