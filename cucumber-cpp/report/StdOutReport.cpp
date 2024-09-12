#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

#ifdef _MSC_VER
#include <Windows.h>
#include <consoleapi2.h>
#include <minwindef.h>
#include <processenv.h>
#include <winbase.h>

#endif

namespace cucumber_cpp::report
{
    namespace
    {
#ifndef _MSC_VER
        inline std::ostream& TcRed(std::ostream& o)
        {
            o << "\033[1m\033[31m";
            return o;
        }

        inline std::ostream& TcGreen(std::ostream& o)
        {
            o << "\033[1m\033[32m";
            return o;
        }

        inline std::ostream& TcCyan(std::ostream& o)
        {
            o << "\033[1m\033[36m";
            return o;
        }

        inline std::ostream& TcDefault(std::ostream& o)
        {
            o << "\033[0m\033[39m";
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

        const std::map<StepType, std::string> stepTypeLut{
            { StepType::given, "Given" },
            { StepType::when, "When" },
            { StepType::then, "Then" }
        };

        std::string ScaledDuration(TraceTime::Duration duration)
        {
            std::ostringstream out;

            if (duration < std::chrono::microseconds{ 1 })
                out << std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
            else if (duration < std::chrono::milliseconds{ 1 })
                out << std::chrono::duration_cast<std::chrono::microseconds>(duration);
            else if (duration < std::chrono::seconds{ 1 })
                out << std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            else if (duration < std::chrono::minutes{ 1 })
                out << std::chrono::duration_cast<std::chrono::seconds>(duration);
            else if (duration < std::chrono::hours{ 1 })
                out << std::chrono::duration_cast<std::chrono::minutes>(duration);
            else
                out << std::chrono::duration_cast<std::chrono::hours>(duration);

            return out.str();
        }
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
        std::cout << "\n"
                  << ruleInfo.Title();
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

    void StdOutReport::ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration /*duration*/)
    {
        std::cout << "\n";

        if (result != engine::Result::passed)
        {
            failedScenarios.emplace_back(&scenarioInfo);
        }
    }

    void StdOutReport::StepSkipped(const engine::StepInfo& stepInfo)
    {
        std::cout << "\n"
                  << TcCyan;
        std::cout << successLut.at(engine::Result::skipped) << " " << stepTypeLut.at(stepInfo.Type()) << " " << stepInfo.Text();
        std::cout << TcDefault;
    }

    void StdOutReport::StepStart(const engine::StepInfo& stepInfo)
    {
        std::cout << "\n"
                  << stepTypeLut.at(stepInfo.Type()) << " " << stepInfo.Text();
    }

    void StdOutReport::StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration)
    {
        using enum engine::Result;

        if (result == passed)
            std::cout << TcGreen;
        else
            std::cout << TcRed;

        std::cout << "\n -> " << successLut.at(result);

        if (result != passed)
            std::cout << " " << stepInfo.ScenarioInfo().FeatureInfo().Path() << ":" << stepInfo.Line() << ":" << stepInfo.Column();

        std::cout << " (" << ScaledDuration(duration) << ")";

        std::cout << TcDefault;
    }

    void StdOutReport::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        std::cout << TcRed;

        if (path && line && column)
            std::cout << "\n"
                      << path.value().string() << ":" << line.value() << ":" << column.value() << ": Failure";

        std::cout << "\n"
                  << error;

        std::cout << TcDefault;
    }

    void StdOutReport::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        std::cout << TcRed;

        if (path && line && column)
            std::cout << "\n"
                      << path.value().string() << ":" << line.value() << ":" << column.value() << ": Error";

        std::cout
            << "\n"
            << error;

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
