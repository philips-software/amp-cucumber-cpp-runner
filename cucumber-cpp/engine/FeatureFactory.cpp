#include "cucumber-cpp/engine/FeatureFactory.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/tag.hpp"
#include <functional>
#include <ranges>
#include <vector>

namespace cucumber_cpp::engine
{
    namespace
    {
        const std::map<cucumber::messages::pickle_step_type, StepType> stepTypeLut{
            { cucumber::messages::pickle_step_type::CONTEXT, StepType::given },
            { cucumber::messages::pickle_step_type::ACTION, StepType::when },
            { cucumber::messages::pickle_step_type::OUTCOME, StepType::then },
        };

        auto TableFactory(const std::optional<cucumber::messages::pickle_step_argument>& optionalPickleStepArgument)
        {
            std::vector<std::vector<TableValue>> table{};

            if (!optionalPickleStepArgument)
                return table;

            if (!optionalPickleStepArgument.value().data_table)
                return table;

            for (const auto& dataTable = optionalPickleStepArgument.value().data_table.value();
                 const auto& row : dataTable.rows)
            {
                table.emplace_back();
                auto& back = table.back();

                for (const auto& cols : row.cells)
                    back.emplace_back(cols.value);
            }

            return table;
        }

        std::vector<std::string> TagsFactory(const std::vector<cucumber::messages::tag>& tags)
        {
            const auto range = tags | std::views::transform(&cucumber::messages::tag::name);
            return { range.begin(), range.end() };
        }

        std::vector<std::string> TagsFactory(const std::vector<cucumber::messages::pickle_tag>& tags)
        {
            const auto range = tags | std::views::transform(&cucumber::messages::pickle_tag::name);
            return { range.begin(), range.end() };
        }

        void ConstructStep(ScenarioInfo& scenarioInfo, const cucumber::messages::step& step, const cucumber::messages::pickle_step& pickleStep)
        {
            std::cout << "\nConstructStep:" << pickleStep.text;
            auto table = TableFactory(pickleStep.argument);

            try
            {
                auto stepMatch = StepRegistry::Instance().Query(stepTypeLut.at(*pickleStep.type), pickleStep.text);

                scenarioInfo.Children().emplace_back(scenarioInfo, pickleStep.text, step.location.line, step.location.column.value_or(0), std::move(table), std::move(stepMatch));
            }
            catch (const StepRegistry::StepNotFoundError&)
            {
                scenarioInfo.MissingChildren().emplace_back(scenarioInfo, pickleStep.text, step.location.line, step.location.column.value_or(0), std::move(table));
            }
            catch (const StepRegistry::AmbiguousStepError& ase)
            {
                scenarioInfo.AmbiguousChildren().emplace_back(scenarioInfo, pickleStep.text, step.location.line, step.location.column.value_or(0), std::move(table), ase.matches);
            }
        }

        void ConstructSteps(ScenarioInfo& scenarioInfo, const std::vector<cucumber::messages::step>& steps, const std::vector<cucumber::messages::pickle_step>& pickleSteps)
        {
            std::cout << "\nConstructSteps";

            for (const auto& pickleStep : pickleSteps)
                for (const auto& step : steps)
                    if (step.id == pickleStep.ast_node_ids[0])
                    {
                        ConstructStep(scenarioInfo, step, pickleStep);
                        break;
                    }
        }

        void ConstructScenario(FeatureInfo& featureInfo, const cucumber::messages::scenario& scenario, const cucumber::messages::pickle& pickle)
        {
            std::cout << "\nConstructScenario.scenario:" << pickle.name;

            ScenarioInfo& scenarioInfo = featureInfo.Children().emplace_back(
                featureInfo,
                TagsFactory(pickle.tags),
                pickle.name,
                scenario.description,
                scenario.location.line,
                scenario.location.column.value_or(0));

            ConstructSteps(scenarioInfo, scenario.steps, pickle.steps);
        }

        void ConstructScenario(FeatureInfo& featureInfo, const cucumber::messages::background& background, const cucumber::messages::pickle& pickle)
        {
            std::cout << "\nConstructScenario.background:" << pickle.name;

            ScenarioInfo& scenarioInfo = featureInfo.Children().emplace_back(
                featureInfo,
                TagsFactory(pickle.tags),
                pickle.name,
                background.description,
                background.location.line,
                background.location.column.value_or(0));

            ConstructSteps(scenarioInfo, background.steps, pickle.steps);
        }

        void ConstructScenario(FeatureInfo& featureInfo, const cucumber::messages::feature& feature, const cucumber::messages::pickle& pickle)
        {
            std::cout << "\nConstructScenario";

            for (const auto& child : feature.children)
                if (child.scenario && child.scenario->id == pickle.ast_node_ids[0])
                    ConstructScenario(featureInfo, *child.scenario, pickle);
                else if (child.background && child.background->id == pickle.ast_node_ids[0])
                    ConstructScenario(featureInfo, *child.background, pickle);
        }

        FeatureInfo FeatureFactory(std::filesystem::path path, const cucumber::gherkin::app::parser_result& ast)
        {
            std::cout << "\nFeatureFactory:" << ast.feature->name;

            return FeatureInfo{
                TagsFactory(ast.feature->tags),
                ast.feature->name,
                ast.feature->description,
                std::move(path),
                ast.feature->location.line,
                ast.feature->location.column.value_or(0)
            };
        }
    }

    FeatureInfo FeatureTreeFactory::Create(const std::filesystem::path& path)
    {
        std::optional<FeatureInfo> featureInfo;
        std::optional<std::reference_wrapper<const cucumber::gherkin::app::parser_result>> parserResult;

        cucumber::gherkin::app::callbacks callbacks{
            .ast = [&path, &parserResult, &featureInfo](const cucumber::gherkin::app::parser_result& ast)
            {
                parserResult = ast;
                featureInfo = FeatureFactory(path, ast);
            },
            .pickle = [&featureInfo, &parserResult](const cucumber::messages::pickle& pickle)
            {
                ConstructScenario(*featureInfo, *(*parserResult).get().feature, pickle);
            },
            .error = [](const cucumber::gherkin::parse_error& /* _ */)
            {
                /* not handled yet */
            }
        };

        gherkin.parse(cucumber::gherkin::file{ path.string() }, callbacks);

        return std::move(*featureInfo);
    }
}
