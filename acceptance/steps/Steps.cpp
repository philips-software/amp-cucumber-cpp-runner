#include "cucumber_cpp/Steps.hpp"
#include "acceptance/util/Subprocess.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cucumber/messages/TestCaseFinished.hpp>
#include <cucumber/messages/TestCaseStarted.hpp>
#include <cucumber/query/EnvelopeArchive.hpp>
#include <filesystem>
#include <fstream>
#include <ios>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
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

    [[nodiscard]] std::filesystem::path LibraryPathFor(const std::filesystem::path& workspace, const std::string& libraryName)
    {
        const auto validatedName = ValidateLibraryName(ReplaceSpacesWithUnderscores(libraryName));
        return workspace / (validatedName + std::string{ DynamicLibrary::PlatformExtension() });
    }

    // Executables live in a subdirectory so `--load <workspace>` never picks up runtime DLLs copied next to them
    [[nodiscard]] std::filesystem::path ExecutablePathFor(const std::filesystem::path& workspace, const std::string& executableName)
    {
        const auto validatedName = ValidateLibraryName(ReplaceSpacesWithUnderscores(executableName));
        return workspace / "bin" / (validatedName + CCR_ACCEPTANCE_HOST_SUFFIX);
    }

    [[nodiscard]] std::filesystem::path WriteScenarioSource(const std::filesystem::path& workspace, const std::string& name, std::string_view content)
    {
        const auto sourcePath = workspace / (ValidateLibraryName(ReplaceSpacesWithUnderscores(name)) + ".cpp");
        std::ofstream sourceFile(sourcePath, std::ios::binary);
        if (!sourceFile.is_open())
            throw std::runtime_error{ "Could not write source: " + sourcePath.string() };

        sourceFile << content;
        return sourcePath;
    }

    struct SandboxTarget
    {
        std::string_view source;
        std::string_view target;
        std::string_view artifact;
        std::string_view runtimeDlls;
    };

    constexpr SandboxTarget pluginSandbox{
        CCR_ACCEPTANCE_SANDBOX_SOURCE,
        CCR_ACCEPTANCE_SANDBOX_TARGET,
        CCR_ACCEPTANCE_SANDBOX_ARTIFACT,
        "",
    };

    constexpr SandboxTarget hostSandbox{
        CCR_ACCEPTANCE_HOST_SOURCE,
        CCR_ACCEPTANCE_HOST_TARGET,
        CCR_ACCEPTANCE_HOST_ARTIFACT,
        CCR_ACCEPTANCE_HOST_RUNTIME_DLLS,
    };

    // Builds sourcePath through the shared sandbox build tree and copies the artifact to outputPath
    [[nodiscard]] ProcessResult BuildInSandbox(const SandboxTarget& sandbox, const std::filesystem::path& sourcePath, const std::filesystem::path& outputPath, const std::filesystem::path& workspace)
    {
        return RunProcess(
            std::filesystem::path{ CCR_ACCEPTANCE_CMAKE_COMMAND },
            {
                "-D",
                "SOURCE=" + sourcePath.string(),
                "-D",
                "OUTPUT=" + outputPath.string(),
                "-D",
                "SANDBOX_SOURCE=" + std::string{ sandbox.source },
                "-D",
                "SANDBOX_TARGET=" + std::string{ sandbox.target },
                "-D",
                "ARTIFACT=" + std::string{ sandbox.artifact },
                "-D",
                "RUNTIME_DLLS=" + std::string{ sandbox.runtimeDlls },
                "-D",
                "BUILD_DIR=" CCR_ACCEPTANCE_BUILD_DIR,
                "-D",
                "CONFIG=" CCR_ACCEPTANCE_BUILD_CONFIG,
                "-D",
                "LOCK_FILE=" CCR_ACCEPTANCE_SANDBOX_LOCK,
                "-P",
                CCR_ACCEPTANCE_SANDBOX_BUILD_SCRIPT,
            },
            workspace);
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

    [[nodiscard]] std::string CombinedOutput(const ProcessResult& result)
    {
        return result.standardOutput + result.standardError;
    }

    void AppendTableArguments(const cucumber_cpp::library::util::Table& table, std::vector<std::string>& arguments)
    {
        for (const auto& row : table.rows)
            arguments.push_back(row.cells.at(0).value);
    }

    [[nodiscard]] ProcessResult RunCli(const std::filesystem::path& workspace, const std::vector<std::string>& arguments)
    {
        return RunProcess(std::filesystem::path{ CCR_ACCEPTANCE_CLI }, arguments, workspace);
    }

    template<class OnEnvelope>
    void LoadMessageEnvelopes(const std::filesystem::path& messagePath, const OnEnvelope& onEnvelope)
    {
        std::ifstream input(messagePath, std::ios::binary);
        if (!input.is_open())
            throw MessageFormatterOutputError{ "Could not read message formatter output: " + messagePath.string() };

        std::string line;
        while (std::getline(input, line))
        {
            if (line.empty())
                continue;

            onEnvelope(nlohmann::json::parse(line).get<cucumber::messages::Envelope>());
        }
    }

    [[nodiscard]] cucumber::query::Query LoadQueryFromMessageFile(const std::filesystem::path& messagePath, cucumber::query::EnvelopeArchive& envelopeArchive)
    {
        cucumber::query::Query query;
        LoadMessageEnvelopes(messagePath, [&query, &envelopeArchive](cucumber::messages::Envelope envelope)
            {
                query.Update(envelopeArchive.Store(std::move(envelope)));
            });
        return query;
    }

    [[nodiscard]] std::vector<const cucumber::messages::TestCaseStarted*> FindScenarioAttempts(const cucumber::query::Query& query, std::string_view scenarioName)
    {
        std::vector<const cucumber::messages::TestCaseStarted*> attempts;
        std::set<std::string> attemptIds;

        const auto collectAttempt = [&query, scenarioName, &attemptIds, &attempts](const cucumber::messages::TestCaseStarted* start)
        {
            if (start == nullptr)
                return;

            const auto* pickle = query.FindPickleBy(*start);
            if (pickle != nullptr && pickle->name == scenarioName && attemptIds.insert(start->id).second)
                attempts.push_back(start);
        };

        for (const auto& step : query.FindAllTestStepStarted())
            collectAttempt(query.FindTestCaseStartedBy(step));

        for (const auto& start : query.FindAllTestCaseStarted())
            collectAttempt(&start);

        std::ranges::sort(attempts, {}, &cucumber::messages::TestCaseStarted::attempt);
        return attempts;
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

    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto outputPath = LibraryPathFor(workspace, libraryName);

    // Write the scenario's plugin source to its own workspace; the sandbox build script copies it into the shared sandbox
    const auto sourcePath = WriteScenarioSource(workspace, libraryName, docString->content);
    const auto buildResult = BuildInSandbox(pluginSandbox, sourcePath, outputPath, workspace);

    context.InsertAt<ProcessResult>(lastBuildResult, buildResult);

    ASSERT_THAT(buildResult.exitCode, testing::Eq(0))
        << "Sandbox build failed\n"
        << FormatProcessResult(buildResult);
}

GIVEN(R"(an executable named {string} with:)", (const std::string& executableName))
{
    if (!docString.has_value())
        FAIL() << "Expected a doc string";

    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto outputPath = ExecutablePathFor(workspace, executableName);
    std::filesystem::create_directories(outputPath.parent_path());

    const auto sourcePath = WriteScenarioSource(workspace, executableName, docString->content);
    const auto buildResult = BuildInSandbox(hostSandbox, sourcePath, outputPath, workspace);

    context.InsertAt<ProcessResult>(lastBuildResult, buildResult);

    ASSERT_THAT(buildResult.exitCode, testing::Eq(0))
        << "Sandbox host build failed\n"
        << FormatProcessResult(buildResult);
}

WHEN(R"(I run cucumber-cpp-runner with {string})", (const std::string& libraryName))
{
    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto libraryPath = LibraryPathFor(workspace, libraryName);
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
}

WHEN(R"(I run cucumber-cpp-runner with {string} and arguments:)", (const std::string& libraryName))
{
    ASSERT_THAT(dataTable, testing::IsTrue());

    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto libraryPath = LibraryPathFor(workspace, libraryName);

    ASSERT_THAT(std::filesystem::is_regular_file(libraryPath), testing::IsTrue())
        << "Expected plugin library at: " << libraryPath;

    std::vector<std::string> arguments{ "--load", libraryPath.string() };
    AppendTableArguments(*dataTable, arguments);

    context.Insert<ProcessResult>(RunCli(workspace, arguments));
}

WHEN(R"(I run the executable {string} with arguments:)", (const std::string& executableName))
{
    if (!dataTable.has_value())
        FAIL() << "Expected a data table";

    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto executablePath = ExecutablePathFor(workspace, executableName);

    ASSERT_THAT(std::filesystem::is_regular_file(executablePath), testing::IsTrue())
        << "Expected executable at: " << executablePath;

    std::vector<std::string> arguments;
    AppendTableArguments(*dataTable, arguments);

    context.Insert<ProcessResult>(RunProcess(executablePath, arguments, workspace));
}

WHEN(R"(I run cucumber-cpp-runner with arguments:)")
{
    ASSERT_THAT(dataTable, testing::IsTrue());

    std::vector<std::string> arguments;
    AppendTableArguments(*dataTable, arguments);

    context.Insert<ProcessResult>(RunCli(context.Get<std::filesystem::path>(scenarioWorkspace), arguments));
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

THEN("the output contains {string}", (const std::string& expected))
{
    const auto& processResult = context.Get<ProcessResult>();
    ASSERT_THAT(CombinedOutput(processResult), testing::HasSubstr(expected)) << FormatProcessResult(processResult);
}

THEN("the output contains:")
{
    ASSERT_THAT(docString, testing::IsTrue());

    const auto& processResult = context.Get<ProcessResult>();
    ASSERT_THAT(CombinedOutput(processResult), testing::HasSubstr(docString->content)) << FormatProcessResult(processResult);
}

THEN("the output does not contain {string}", (const std::string& unexpected))
{
    const auto& processResult = context.Get<ProcessResult>();
    ASSERT_THAT(CombinedOutput(processResult), testing::Not(testing::HasSubstr(unexpected))) << FormatProcessResult(processResult);
}

THEN("the message file {string} contains {int} attempts for scenario {string}", (const std::string& filename, std::int32_t expectedCount, const std::string& scenarioName))
{
    const auto workspace = context.Get<std::filesystem::path>(scenarioWorkspace);
    const auto query = LoadQueryFromMessageFile(workspace / ValidateRelativePath(filename), *context.Emplace<cucumber::query::EnvelopeArchive>());
    const auto attempts = FindScenarioAttempts(query, scenarioName);

    ASSERT_THAT(attempts.size(), testing::Eq(expectedCount));

    for (std::size_t attempt = 0; attempt < attempts.size(); ++attempt)
    {
        const auto& start = *attempts.at(attempt);
        const auto* finish = query.FindTestCaseFinishedBy(start);

        ASSERT_THAT(finish, testing::NotNull());
        EXPECT_THAT(start.attempt, testing::Eq(attempt));
        EXPECT_THAT(start.testCaseId, testing::Eq(attempts.front()->testCaseId));
        EXPECT_THAT(finish->willBeRetried, testing::Eq(attempt + 1 < attempts.size()));
    }
}
