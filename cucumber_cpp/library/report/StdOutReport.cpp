#include "cucumber_cpp/library/report/StdOutReport.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#ifdef _MSC_VER
// clang-format off
#include <Windows.h>
#include <consoleapi2.h>
#include <minwindef.h>
#include <processenv.h>
#include <WinBase.h>
#include <winnls.h>
// clang-format on
#endif

namespace cucumber_cpp::library::report
{
    namespace
    {
#ifndef _MSC_VER
        constexpr auto redStr{ "\o{33}[1m\o{33}[31m" };
        constexpr auto greenStr{ "\o{33}[1m\o{33}[32m" };
        constexpr auto cyanStr{ "\o{33}[1m\o{33}[36m" };
        constexpr auto defaultStr{ "\o{33}[0m\o{33}[39m" };

        inline std::ostream& TcRed(std::ostream& o)
        {
            o << redStr;
            return o;
        }

        inline std::ostream& TcGreen(std::ostream& o)
        {
            o << greenStr;
            return o;
        }

        inline std::ostream& TcCyan(std::ostream& o)
        {
            o << cyanStr;
            return o;
        }

        inline std::ostream& TcDefault(std::ostream& o)
        {
            o << defaultStr;
            return o;
        }
#else
        WORD GetDefaultConsoleValue()
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
            return info.wAttributes;
        }

        WORD GetDefaultConsole()
        {
            static WORD defaultValue = GetDefaultConsoleValue();
            return defaultValue;
        }

        void SetColorConsole(WORD color)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color | (GetDefaultConsole() & ~(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)));
        }

        inline std::ostream& TcRed(std::ostream& o)
        {
            SetColorConsole(FOREGROUND_INTENSITY | FOREGROUND_RED);
            return o;
        }

        inline std::ostream& TcGreen(std::ostream& o)
        {
            SetColorConsole(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
            return o;
        }

        inline std::ostream& TcCyan(std::ostream& o)
        {
            SetColorConsole(FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
            return o;
        }

        inline std::ostream& TcDefault(std::ostream& o)
        {
            SetColorConsole(GetDefaultConsole());
            return o;
        }
#endif

        const std::map<engine::Result, std::string> successLut{
            { engine::Result ::passed, "done" },
            { engine::Result ::skipped, "skipped" },
            { engine::Result ::failed, "failed" },
            { engine::Result ::pending, "pending" },
            { engine::Result ::ambiguous, "ambiguous" },
            { engine::Result ::undefined, "undefined" },
        };

        const std::map<engine::StepType, std::string> stepTypeLut{
            { engine::StepType::given, "Given" },
            { engine::StepType::when, "When" },
            { engine::StepType::then, "Then" }
        };

        std::string ScaledDuration(TraceTime::Duration duration)
        {
            std::ostringstream out;

            if (duration < std::chrono::microseconds{ 1 })
                out << std::chrono::duration<double, std::chrono::nanoseconds::period>(duration);
            else if (duration < std::chrono::milliseconds{ 1 })
                out << std::chrono::duration<double, std::chrono::microseconds::period>(duration);
            else if (duration < std::chrono::seconds{ 1 })
                out << std::chrono::duration<double, std::chrono::milliseconds::period>(duration);
            else if (duration < std::chrono::minutes{ 1 })
                out << std::chrono::duration<double, std::chrono::seconds::period>(duration);
            else if (duration < std::chrono::hours{ 1 })
                out << std::chrono::duration<double, std::chrono::minutes::period>(duration);
            else
                out << std::chrono::duration<double, std::chrono::hours::period>(duration);

            return out.str();
        }

        std::string Repeat(std::string_view input, std::size_t count)
        {
            std::string result;
            result.reserve(input.size() * count);
            for (std::size_t i = 0; i < count; ++i)
                result += input;
            return result;
        }
    }

    StdOutReport::StdOutReport()
    {
#ifdef _MSC_VER
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
    }

    void StdOutReport::FeatureStart(const engine::FeatureInfo& featureInfo)
    {
        // not required
    }

    void StdOutReport::FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration)
    {
        // not required
    }

    void StdOutReport::RuleStart(const engine::RuleInfo& ruleInfo)
    {
        // not required
    }

    void StdOutReport::RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration)
    {
        // not required
    }

    void StdOutReport::ScenarioStart(const engine::ScenarioInfo& scenarioInfo)
    {
        ++nrOfScenarios;
        std::cout << "\n"
                  << scenarioInfo.Title();
    }

    void StdOutReport::ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration)
    {
        using enum engine::Result;

        std::cout << "\n"
                  << "\\-> ";

        auto& coloured = ((result == passed) ? std::cout << TcGreen : std::cout << TcRed);

        coloured << successLut.at(result)
                 << " (" << ScaledDuration(duration) << ")"
                 << TcDefault << '\n';

        if (result != engine::Result::passed)
        {
            failedScenarios.emplace_back(&scenarioInfo);
        }
    }

    void StdOutReport::StepMissing(const std::string& stepText)
    {
        std::cout << "\n"
                  << std::format(R"({}Step missing: "{}")", redStr, stepText);
        std::cout << TcDefault;
    }

    void StdOutReport::StepAmbiguous(const std::string& stepText, const engine::StepInfo& stepInfo)
    {
        std::cout << "\n"
                  << std::format(R"({}Ambiguous step: "{}" Matches:)", redStr, stepText);

        for (const auto& match : std::get<std::vector<StepRegistry::StepMatch>>(stepInfo.StepMatch()))
        {
            std::visit([&match](const auto& pattern)
                {
                    std::cout << "\n"
                              << std::format(R"({}:{}:{} : {})", match.entry.loc.file_name(), match.entry.loc.line(), match.entry.loc.column(), pattern.Source());
                },
                match.entry.regex);
        }
        std::cout << TcDefault;
    }

    void StdOutReport::StepSkipped(const engine::StepInfo& stepInfo)
    {
        std::cout << "\n"
                  << Repeat("| ", nestedSteps)
                  << TcCyan;
        std::cout << successLut.at(engine::Result::skipped) << " " << stepTypeLut.at(stepInfo.Type()) << " " << stepInfo.Text();
        std::cout << TcDefault;
    }

    void StdOutReport::StepStart(const engine::StepInfo& stepInfo)
    {
        std::cout << "\n"
                  << Repeat("| ", nestedSteps)
                  << stepTypeLut.at(stepInfo.Type()) << " " << stepInfo.Text()
                  << std::flush;
        ++nestedSteps;
    }

    void StdOutReport::StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration)
    {
        --nestedSteps;

        using enum engine::Result;

        std::cout << "\n"
                  << Repeat("| ", nestedSteps) << "\\-> ";

        if (result == passed)
            std::cout << TcGreen;
        else
            std::cout << TcRed;

        std::cout << successLut.at(result);

        if (result != passed)
            std::cout << " " << stepInfo.ScenarioInfo().FeatureInfo().Path() << ":" << stepInfo.Line() << ":" << stepInfo.Column();

        std::cout << " (" << ScaledDuration(duration) << ")";

        std::cout << TcDefault;
    }

    void StdOutReport::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        std::cout << TcRed;

        if (path && line && column)
            std::cout << std::format("\nFailure @ ./{}:{}:{}:", path.value().string(), line.value(), column.value());
        else if (path && line)
            std::cout << std::format("\nFailure @ ./{}:{}:", path.value().string(), line.value());

        std::cout << std::format("\n{}", error);

        std::cout << TcDefault;
    }

    void StdOutReport::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        std::cout << TcRed;

        if (path && line && column)
            std::cout << std::format("\nError @ ./{}:{}:{}:", path.value().string(), line.value(), column.value());
        else if (path && line)
            std::cout << std::format("\nError @ ./{}:{}:", path.value().string(), line.value());

        std::cout << std::format("\n{}", error);

        std::cout << TcDefault;
    }

    void StdOutReport::Trace(const std::string& trace)
    {
        std::cout << trace;
    }

    void StdOutReport::Summary(TraceTime::Duration duration)
    {
        std::cout << "\n====================summary====================";
        std::cout << "\nduration: " << ScaledDuration(duration);
        std::cout << "\ntests   : " << (nrOfScenarios - failedScenarios.size()) << "/" << nrOfScenarios << " passed";

        if (!failedScenarios.empty())
        {
            std::cout << "\n\nfailed tests:";

            for (const auto* scenarioInfo : failedScenarios)
                std::cout << "\n"
                          << scenarioInfo->Path() << ":" << scenarioInfo->Line() << ":" << scenarioInfo->Column() << " : " << std::quoted(scenarioInfo->Title());
        }
    }
}
