#include "cucumber-cpp/report/StdOutReport.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <iostream>
#include <ranges>

namespace cucumber_cpp::report
{
    namespace
    {
        void PrintLineWithResult(const std::string& text, const std::string& result, std::uint32_t indent, std::uint32_t spacing)
        {
            const auto size = text.size() + indent;
            auto dots = std::max(static_cast<int>(spacing - size), 5);

            std::cout << "\n";
            for (; indent != 0; --indent)
                std::cout << ' ';

            std::cout << text;
            for (; dots != 0; --dots)
                std::cout << '.';

            std::cout << result;
        }

        void PrintTable(const nlohmann::json& step)
        {
            if (step.contains("argument") && !step["argument"]["dataTable"].empty())
            {
                const auto& table = step["argument"]["dataTable"];

                auto colSize = [&table](int colIndex)
                {
                    return std::ranges::max(table["rows"] | std::views::transform(
                                                                [colIndex](const nlohmann::json& row)
                                                                {
                                                                    return row["cells"][colIndex]["value"].get<std::string>().size();
                                                                }));
                };

                for (auto row : table["rows"])
                {
                    std::cout << "\n        |";
                    auto colIndex{ 0 };
                    for (auto col : row["cells"])
                    {
                        const auto value = col["value"].get<std::string>();
                        std::cout << " " << value << std::string(colSize(colIndex) - value.size(), ' ');
                        std::cout << " |";
                        ++colIndex;
                    }
                }
            }
        }

        const std::map<std::string_view, std::string_view> stepTypeLut{
            { "Context", "Given" },
            { "Action", "When" },
            { "Outcome", "Then" },
        };

        std::string CreateStepText(const nlohmann::json& stepJson)
        {
            return std::string{ stepTypeLut.at(stepJson["type"].get<std::string>()) } + " " + stepJson["text"].get<std::string>();
        }

        double RoundTo(double value, double precision)
        {
            return std::round(value / precision) * precision;
        }
    }

    void StdOutReport::GenerateReport(const nlohmann::json& json)
    {
        for (const auto& feature : json["features"])
        {
            for (const auto& scenario : feature["scenarios"])
            {
                if (!scenario.contains("result"))
                {
                    continue;
                }

                std::cout << "\n";
                PrintLineWithResult(scenario["name"], scenario["result"], 0, 60);
                std::cout << " " << RoundTo(scenario.value("elapsed", 0.0), 0.001) << "s";

                for (const auto& step : scenario["steps"])
                {
                    if (!step.contains("result"))
                    {
                        continue;
                    }

                    PrintLineWithResult(CreateStepText(step), step["result"], 4, 60);
                    std::cout << " " << RoundTo(step.value("elapsed", 0.0), 0.001) << "s";
                    PrintTable(step);

                    if (step.contains("stdout"))
                    {
                        std::cout << "\nstdout:\n"
                                  << step["stdout"].get<std::string>();
                    }

                    if (step.contains("stderr"))
                    {
                        std::cout << "\nstderr:\n"
                                  << step["stderr"].get<std::string>();
                    }
                }
            }
        }
    }
}
