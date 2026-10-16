#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber/cucumber-expressions/RegexStrategy.hpp"
#include "cucumber/cucumber-expressions/RegexStrategyFactory.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/ParameterType.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/StepDefinitionPattern.hpp"
#include "cucumber/messages/StepDefinitionPatternType.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/LoadPickles.hpp"
#include "cucumber_cpp/library/api/MakeUniqueTestNames.hpp"
#include "cucumber_cpp/library/plugin/HookLoader.hpp"
#include "cucumber_cpp/library/plugin/ParameterLoader.hpp"
#include "cucumber_cpp/library/plugin/StepLoader.hpp"
#include "cucumber_cpp/library/runtime/MakeRuntime.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <cucumber/cucumber-expressions/ParameterRegistry.hpp>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        void EmitParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const cucumber::gherkin::IdGeneratorPtr& idGenerator)
        {
            for (const auto& [name, parameter] : supportCodeLibrary.parameterRegistry.GetParameters())
            {
                if (parameter.isBuiltin)
                    continue;

                broadcaster.BroadcastEvent([&parameter, &idGenerator](cucumber::messages::Envelope& envelope)
                    {
                        cucumber::messages::Location location;
                        location.line = parameter.location.line();

                        cucumber::messages::SourceReference sourceReference;
                        sourceReference.uri = parameter.location.file_name();
                        sourceReference.location = location;

                        cucumber::messages::ParameterType parameterType{};
                        parameterType.name = parameter.name;
                        parameterType.regularExpressions = parameter.regex;
                        parameterType.useForSnippets = parameter.useForSnippets;
                        parameterType.id = idGenerator->NextId();
                        parameterType.sourceReference = sourceReference;

                        envelope.parameterType = std::move(parameterType);
                    });
            }
        }

        void EmitUndefinedParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& parameter : supportCodeLibrary.undefinedParameters.definitions)
                broadcaster.BroadcastEvent([&parameter](cucumber::messages::Envelope& envelope)
                    {
                        envelope.undefinedParameterType = parameter;
                    });
        }

        void EmitStepDefinition(const support::StepRegistry::Definition& stepDefinition, util::Broadcaster& broadcaster)
        {
            broadcaster.BroadcastEvent([&stepDefinition](cucumber::messages::Envelope& envelope)
                {
                    cucumber::messages::StepDefinitionPattern pattern;
                    pattern.source = stepDefinition.pattern;
                    pattern.type = stepDefinition.patternType == support::ExpressionPatternType::cucumberExpression ? cucumber::messages::StepDefinitionPatternType::CUCUMBER_EXPRESSION : cucumber::messages::StepDefinitionPatternType::REGULAR_EXPRESSION;

                    cucumber::messages::Location location;
                    location.line = stepDefinition.line;

                    cucumber::messages::SourceReference sourceReference;
                    sourceReference.uri = stepDefinition.uri.string();
                    sourceReference.location = location;

                    cucumber::messages::StepDefinition stepDefinitionMessage;
                    stepDefinitionMessage.id = stepDefinition.id;
                    stepDefinitionMessage.pattern = pattern;
                    stepDefinitionMessage.sourceReference = sourceReference;

                    envelope.stepDefinition = std::move(stepDefinitionMessage);
                });
        }

        void EmitHook(const util::HookData& hook, util::Broadcaster& broadcaster)
        {
            broadcaster.BroadcastEvent([&hook](cucumber::messages::Envelope& envelope)
                {
                    envelope.hook = util::TransformHookData(hook);
                });
        }

        void EmitStepDefinitionsAndHooks(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            // Steps with undefined parameter types are not part of the step registry
            std::map<std::string, const support::StepRegistry::Definition*, std::less<>> stepDefinitions;
            for (const auto& stepDefinition : supportCodeLibrary.stepRegistry.StepDefinitions())
                stepDefinitions.emplace(stepDefinition.id, &stepDefinition);

            support::DefinitionRegistration::Instance().ForEachRegisteredEntry([&](const support::Entry& entry)
                {
                    if (const auto* step = std::get_if<support::StepStringRegistration::Entry>(&entry))
                    {
                        if (const auto iter = stepDefinitions.find(step->id); iter != stepDefinitions.end())
                            EmitStepDefinition(*iter->second, broadcaster);
                    }
                    else
                        EmitHook(supportCodeLibrary.hookRegistry.GetDefinitionById(std::get<support::HookEntry>(entry).id).data, broadcaster);
                });
        }

        void EmitSupportCodeMessages(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const cucumber::gherkin::IdGeneratorPtr& idGenerator)
        {
            // Phase 1: Load parameters (must be first, steps reference parameter types)
            plugin::ParameterLoader::Load(support::DefinitionRegistration::Instance(), supportCodeLibrary.parameterRegistry);
            EmitParameters(supportCodeLibrary, broadcaster, idGenerator);

            support::DefinitionRegistration::Instance().LoadIds(idGenerator);

            // Phase 2: Load steps (can now resolve parameter type expressions)
            plugin::StepLoader::Load(supportCodeLibrary.stepRegistry);
            plugin::HookLoader::Load(supportCodeLibrary.hookRegistry);

            EmitUndefinedParameters(supportCodeLibrary, broadcaster);
            EmitStepDefinitionsAndHooks(supportCodeLibrary, broadcaster);
        }

        using RegexStrategies = std::vector<std::unique_ptr<cucumber::cucumber_expressions::RegexStrategy>>;

        std::optional<RegexStrategies> CreateNameRegexStrategies(const std::vector<std::string>& nameRegexes)
        {
            RegexStrategies strategies;
            bool allValid = true;

            for (const auto& pattern : nameRegexes)
            {
                try
                {
                    strategies.push_back(cucumber::cucumber_expressions::CreateRegexStrategy(pattern));
                }
                catch (const std::exception& e)
                {
                    fmt::println(std::cerr, "Invalid regex pattern '{}': {}", pattern, e.what());
                    allValid = false;
                }
            }

            if (!allValid)
                return std::nullopt;

            return strategies;
        }

        auto FilterByNameRegex(const RegexStrategies& strategies, const std::map<std::string, std::string, std::less<>>& uniqueNames)
        {
            return [&strategies, &uniqueNames](const support::PickleSource& pickleSource)
            {
                if (strategies.empty())
                    return true;

                const auto& pickle = *pickleSource.pickle;
                const auto& fullName = uniqueNames.at(pickle.id);

                return std::ranges::any_of(strategies, [&pickle, &fullName](const auto& strategy)
                    {
                        return strategy->Match(pickle.name).has_value() || strategy->Match(fullName).has_value();
                    });
            };
        }

        std::list<support::PickleSource> OrderPickles(const support::RunOptions::Sources& sources, auto pickles)
        {
            const auto createOrderedPickleList = [](auto ordered) -> std::list<support::PickleSource>
            {
                return { std::begin(ordered), std::end(ordered) };
            };

            if (sources.ordering == support::RunOptions::Ordering::defined)
                return createOrderedPickleList(std::move(pickles));
            else
                return createOrderedPickleList(std::move(pickles) | std::views::reverse);
        };
    }

    bool RunCucumber(const support::RunOptions& options, cucumber::cucumber_expressions::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string, std::less<>>& format, const std::string& formatOptions)
    {
        cucumber::gherkin::IdGeneratorPtr idGenerator = std::make_shared<cucumber::gherkin::IdGenerator>();

        support::UndefinedParameters undefinedParameters;
        support::StepRegistry stepRegistry{ parameterRegistry, undefinedParameters, idGenerator };
        support::HookRegistry hookRegistry{ idGenerator };

        support::SupportCodeLibrary supportCodeLibrary{
            .hookRegistry = hookRegistry,
            .stepRegistry = stepRegistry,
            .parameterRegistry = parameterRegistry,
            .undefinedParameters = undefinedParameters,
        };

        const auto formatOptionsJson = formatOptions.empty() ? nlohmann::json::object() : nlohmann::json::parse(formatOptions);
        const auto activeFormatters = formatters.EnableFormatters(format, formatOptionsJson, supportCodeLibrary, broadcaster);

        cucumber::query::Query query;
        util::Listener queryListener{ broadcaster, [&query](const cucumber::messages::Envelope& envelope)
            {
                query.Update(envelope);
            } };

        const auto nameRegexStrategies = CreateNameRegexStrategies(options.sources.nameRegexes);
        if (!nameRegexStrategies)
            return false;

        const auto pickleSources = LoadPickles(options.sources, idGenerator, broadcaster);
        if (!pickleSources)
            return false;

        // Unique names are derived in defined order, before reordering, to match the names reported by ListScenarios
        const auto uniqueNames = MakeUniqueTestNames(query, *pickleSources);
        const auto orderedPickles = OrderPickles(options.sources, *pickleSources | std::views::filter(FilterByNameRegex(*nameRegexStrategies, uniqueNames)));

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        const auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, orderedPickles, supportCodeLibrary, idGenerator, programContext);
        return runtime->Run();
    }
}
