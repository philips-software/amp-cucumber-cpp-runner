#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/gherkin/file.hpp"
#include "cucumber/gherkin/parse_error.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/pickle_step_argument.hpp"
#include "cucumber/messages/pickle_step_type.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/TagExpression.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        struct ScenarioWithRule
        {
            const cucumber::messages::rule* rule;
            const cucumber::messages::scenario* scenario;
        };

        using AllTypes = std::variant<
            const cucumber::messages::background*,
            const cucumber::messages::scenario*,
            const cucumber::messages::step*,
            ScenarioWithRule>;

        using FlatAst = std::map<std::string, AllTypes, std::less<>>;

        void FlattenBackgroundAst(FlatAst& flatAst, const cucumber::messages::background& background)
        {
            flatAst[background.id] = &background;
            for (const auto& step : background.steps)
                flatAst[step.id] = &step;
        }

        void FlattenScenarioAst(FlatAst& flatAst, const cucumber::messages::scenario& scenario)
        {
            flatAst[scenario.id] = &scenario;
            for (const auto& step : scenario.steps)
                flatAst[step.id] = &step;
        }

        void FlattenRuleAst(FlatAst& flatAst, const cucumber::messages::rule& rule)
        {
            for (const auto& ruleChild : rule.children)
            {
                if (ruleChild.background)
                {
                    flatAst[ruleChild.background->id] = &*ruleChild.background;
                    for (const auto& step : ruleChild.background->steps)
                        flatAst[step.id] = &step;
                }
                else if (ruleChild.scenario)
                {
                    flatAst[ruleChild.scenario->id] = ScenarioWithRule{ &rule, &*ruleChild.scenario };
                    for (const auto& step : ruleChild.scenario->steps)
                        flatAst[step.id] = &step;
                }
            }
        }

        FlatAst FlattenAst(const cucumber::messages::feature& feature)
        {
            FlatAst flatAst;

            for (const auto& child : feature.children)
            {
                if (child.background)
                    FlattenBackgroundAst(flatAst, *child.background);
                else if (child.rule)
                    FlattenRuleAst(flatAst, *child.rule);
                else if (child.scenario)
                    FlattenScenarioAst(flatAst, *child.scenario);
            }

            return flatAst;
        }

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

        std::set<std::string, std::less<>> TagsFactory(const std::vector<cucumber::messages::tag>& tags)
        {
            const auto range = tags | std::views::transform(&cucumber::messages::tag::name);
            return { range.begin(), range.end() };
        }

        std::set<std::string, std::less<>> TagsFactory(const std::vector<cucumber::messages::pickle_tag>& tags)
        {
            const auto range = tags | std::views::transform(&cucumber::messages::pickle_tag::name);
            return { range.begin(), range.end() };
        }

        void ConstructStep(const FeatureTreeFactory& featureTreeFactory, ScenarioInfo& scenarioInfo, const cucumber::messages::step& step, const cucumber::messages::pickle_step& pickleStep)
        {
            auto table = TableFactory(pickleStep.argument);

            try
            {
                scenarioInfo.Children().push_back(featureTreeFactory.CreateStepInfo(stepTypeLut.at(*pickleStep.type), pickleStep.text, scenarioInfo, step.location.line, step.location.column.value_or(0), std::move(table)));
            }
            catch (const std::out_of_range&)
            {

                throw UnsupportedAsteriskError{ std::format("{}:{}: * steps are not supported", scenarioInfo.FeatureInfo().Path().string(), step.location.line) };
            }
        }

        void ConstructSteps(const FeatureTreeFactory& featureTreeFactory, ScenarioInfo& scenarioInfo, const FlatAst& flatAst, const std::vector<cucumber::messages::pickle_step>& pickleSteps)
        {
            for (const auto& pickleStep : pickleSteps)
            {
                const auto* astStep = std::get<const cucumber::messages::step*>(flatAst.at(pickleStep.ast_node_ids.front()));

                ConstructStep(featureTreeFactory, scenarioInfo, *astStep, pickleStep);
            }
        }

        void ConstructScenario(const FeatureTreeFactory& featureTreeFactory, FeatureInfo& featureInfo, const FlatAst& flatAst, const cucumber::messages::scenario& scenario, const cucumber::messages::pickle& pickle, std::string_view tagExpression)
        {
            auto tags = TagsFactory(pickle.tags);

            if (!IsTagExprSelected(tagExpression, tags))
                return;

            featureInfo.Scenarios().push_back(std::make_unique<ScenarioInfo>(
                featureInfo,
                std::move(tags),
                pickle.name,
                scenario.description,
                scenario.location.line,
                scenario.location.column.value_or(0)));

            ConstructSteps(featureTreeFactory, *featureInfo.Scenarios().back(), flatAst, pickle.steps);
        }

        void ConstructScenario(const FeatureTreeFactory& featureTreeFactory, RuleInfo& ruleInfo, const FlatAst& flatAst, const cucumber::messages::scenario& scenario, const cucumber::messages::pickle& pickle, std::set<std::string, std::less<>> tags)
        {
            ruleInfo.Scenarios().push_back(std::make_unique<ScenarioInfo>(
                ruleInfo,
                std::move(tags),
                pickle.name,
                scenario.description,
                scenario.location.line,
                scenario.location.column.value_or(0)));

            ConstructSteps(featureTreeFactory, *ruleInfo.Scenarios().back(), flatAst, pickle.steps);
        }

        RuleInfo& GetOrConstructRule(FeatureInfo& featureInfo, const cucumber::messages::rule& rule)
        {
            if (auto iter = std::ranges::find(featureInfo.Rules(), rule.id, &RuleInfo::Id); iter != featureInfo.Rules().end())
                return **iter;

            featureInfo.Rules().push_back(
                std::make_unique<RuleInfo>(featureInfo,
                    rule.id,
                    rule.name,
                    rule.description,
                    rule.location.line,
                    rule.location.column.value_or(0)));

            return *featureInfo.Rules().back();
        }

        void ConstructScenarioWithRule(const FeatureTreeFactory& featureTreeFactory, FeatureInfo& featureInfo, const FlatAst& flatAst, const cucumber::messages::rule& rule, const cucumber::messages::scenario& scenario, const cucumber::messages::pickle& pickle, std::string_view tagExpression)
        {
            auto tags = TagsFactory(pickle.tags);

            if (!IsTagExprSelected(tagExpression, tags))
                return;

            auto& ruleInfo = GetOrConstructRule(featureInfo, rule);

            ConstructScenario(featureTreeFactory, ruleInfo, flatAst, scenario, pickle, std::move(tags));
        }

        struct ConstructScenarioVisitor
        {
            ConstructScenarioVisitor(const FeatureTreeFactory& featureTreeFactory, FeatureInfo& featureInfo, const FlatAst& flatAst, const cucumber::messages::pickle& pickle, std::string_view tagExpression)
                : featureTreeFactory{ featureTreeFactory }
                , featureInfo{ featureInfo }
                , flatAst{ flatAst }
                , pickle{ pickle }
                , tagExpression{ tagExpression }
            {}

            void operator()(const cucumber::messages::scenario* scenario) const
            {
                ConstructScenario(featureTreeFactory, featureInfo, flatAst, *scenario, pickle, tagExpression);
            }

            void operator()(ScenarioWithRule ruleWithScenario) const
            {
                ConstructScenarioWithRule(featureTreeFactory, featureInfo, flatAst, *ruleWithScenario.rule, *ruleWithScenario.scenario, pickle, tagExpression);
            }

            void operator()(auto /* ignore */) const
            {
                /* ignore */
            }

        private:
            const FeatureTreeFactory& featureTreeFactory;
            FeatureInfo& featureInfo;
            const FlatAst& flatAst;
            const cucumber::messages::pickle& pickle;
            std::string_view tagExpression;
        };

        void ConstructScenario(const FeatureTreeFactory& featureTreeFactory, FeatureInfo& featureInfo, const FlatAst& flatAst, const cucumber::messages::pickle& pickle, std::string_view tagExpression)
        {
            ConstructScenarioVisitor visitor{ featureTreeFactory, featureInfo, flatAst, pickle, tagExpression };

            std::visit(visitor, flatAst.at(pickle.ast_node_ids.front()));
        }

        std::unique_ptr<FeatureInfo> FeatureFactory(std::filesystem::path path, const cucumber::gherkin::app::parser_result& ast)
        {
            return std::make_unique<FeatureInfo>(
                TagsFactory(ast.feature->tags),
                ast.feature->name,
                ast.feature->description,
                std::move(path),
                ast.feature->location.line,
                ast.feature->location.column.value_or(0));
        }
    }

    FeatureTreeFactory::FeatureTreeFactory(StepRegistry& stepRegistry, report::ReportForwarder& reportHandler)
        : stepRegistry{ stepRegistry }
        , reportHandler{ reportHandler }
    {}

    std::unique_ptr<StepInfo> FeatureTreeFactory::CreateStepInfo(StepType stepType, std::string stepText, const ScenarioInfo& scenarioInfo, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table) const
    {
        try
        {
            auto stepMatch = stepRegistry.Query(stepType, stepText);
            return std::make_unique<StepInfo>(scenarioInfo, std::move(stepText), stepType, line, column, std::move(table), std::move(stepMatch));
        }
        catch (const StepRegistry::StepNotFoundError&)
        {
            auto stepInfo = std::make_unique<StepInfo>(scenarioInfo, stepText, stepType, line, column, std::move(table));
            reportHandler.StepMissing(stepText);
            return stepInfo;
        }
        catch (StepRegistry::AmbiguousStepError& ase)
        {
            auto stepInfo = std::make_unique<StepInfo>(scenarioInfo, stepText, stepType, line, column, std::move(table), std::move(ase.matches));
            reportHandler.StepAmbiguous(stepText, *stepInfo);
            return stepInfo;
        }
    }

    std::unique_ptr<FeatureInfo> FeatureTreeFactory::Create(const std::filesystem::path& path, std::string_view tagExpression) const
    {
        std::unique_ptr<FeatureInfo> featureInfo;
        std::optional<FlatAst> flatAst;

        cucumber::gherkin::app::callbacks callbacks{
            .ast = [&path, &flatAst, &featureInfo](const cucumber::gherkin::app::parser_result& ast)
            {
                featureInfo = FeatureFactory(path, ast);
                flatAst = FlattenAst(*ast.feature);
            },
            .pickle = [this, &featureInfo, &flatAst, &tagExpression](const cucumber::messages::pickle& pickle)
            {
                ConstructScenario(*this, *featureInfo, *flatAst, pickle, tagExpression);
            },
            .error = [](const cucumber::gherkin::parse_error& /* _ */)
            {
                /* not handled yet */
            }
        };

        cucumber::gherkin::app gherkin;

        gherkin.parse(cucumber::gherkin::file{ path.string() }, callbacks);

        return featureInfo;
    }
}
