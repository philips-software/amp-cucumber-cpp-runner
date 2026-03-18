#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
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
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        void EmitParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const cucumber::gherkin::id_generator_ptr& idGenerator)
        {
            for (const auto& [name, parameter] : supportCodeLibrary.parameterRegistry.GetParameters())
            {
                if (parameter.isBuiltin)
                    continue;

                broadcaster.BroadcastEvent(cucumber::messages::envelope{
                    .parameter_type = cucumber::messages::parameter_type{
                        .name = parameter.name,
                        .regular_expressions = parameter.regex,
                        .use_for_snippets = parameter.useForSnippets,
                        .id = idGenerator->next_id(),
                        .source_reference = cucumber::messages::source_reference{
                            .uri = parameter.location.file_name(),
                            .location = cucumber::messages::location{
                                .line = parameter.location.line(),
                            },
                        },
                    },
                });
            }
        }

        void EmitUndefinedParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& parameter : supportCodeLibrary.undefinedParameters.definitions)
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .undefined_parameter_type = parameter });
        }

        void EmitStepDefinitions(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& stepDefinition : supportCodeLibrary.stepRegistry.StepDefinitions())
            {
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .step_definition = cucumber::messages::step_definition{
                                                                             .id = stepDefinition.id,
                                                                             .pattern = cucumber::messages::step_definition_pattern{
                                                                                 .source = stepDefinition.pattern,
                                                                                 .type = stepDefinition.patternType == support::ExpressionPatternType::cucumberExpression ? cucumber::messages::step_definition_pattern_type::CUCUMBER_EXPRESSION : cucumber::messages::step_definition_pattern_type::REGULAR_EXPRESSION,
                                                                             },
                                                                             .source_reference = {
                                                                                 .uri = stepDefinition.uri.string(),
                                                                                 .location = cucumber::messages::location{
                                                                                     .line = stepDefinition.line,
                                                                                 },
                                                                             },
                                                                         } });
            }
        }

        void EmitTestCaseHooks(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::before);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .hook = util::TransformHookData(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::after);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .hook = util::TransformHookData(hook) });
        }

        void EmitTestRunHooks(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::beforeAll);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .hook = util::TransformHookData(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::afterAll);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent(cucumber::messages::envelope{ .hook = util::TransformHookData(hook) });
        }

        void EmitSupportCodeMessages(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, const cucumber::gherkin::id_generator_ptr& idGenerator)
        {
            EmitParameters(supportCodeLibrary, broadcaster, idGenerator);

            support::DefinitionRegistration::Instance().LoadIds(idGenerator);
            supportCodeLibrary.stepRegistry.LoadSteps();

            EmitUndefinedParameters(supportCodeLibrary, broadcaster);
            EmitStepDefinitions(supportCodeLibrary, broadcaster);

            supportCodeLibrary.hookRegistry.LoadHooks();
            EmitTestCaseHooks(supportCodeLibrary, broadcaster);
            EmitTestRunHooks(supportCodeLibrary, broadcaster);
        }

        const std::string& TransformPickleTagName(const cucumber::messages::pickle_tag& tag)
        {
            return tag.name;
        }

        std::set<std::string, std::less<>> PickleTagsToSet(const std::vector<cucumber::messages::pickle_tag>& tags)
        {
            auto tagNames = tags | std::views::transform(TransformPickleTagName);
            return { std::begin(tagNames), std::end(tagNames) };
        }

        auto FilterByTagExpression(const support::RunOptions::Sources& sources)
        {
            return [&sources](const support::PickleSource& pickle)
            {
                return sources.tagExpression->Evaluate(PickleTagsToSet(pickle.pickle->tags));
            };
        }

        std::list<support::PickleSource> OrderPickles(const support::RunOptions::Sources& sources, auto pickles)
        {
            const auto createOrderedPickleList = [](auto ordered) -> std::list<support::PickleSource>
            {
                return { std::begin(ordered), std::end(ordered) };
            };

            if (sources.ordering == support::RunOptions::Ordering::defined)
                return createOrderedPickleList(pickles);
            else
                return createOrderedPickleList(pickles | std::views::reverse);
        };
    }

    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string, std::less<>>& format, const std::string& formatOptions)
    {
        cucumber::gherkin::id_generator_ptr idGenerator = std::make_shared<cucumber::gherkin::id_generator>();

        support::UndefinedParameters undefinedParameters;
        support::StepRegistry stepRegistry{ parameterRegistry, undefinedParameters, idGenerator };
        support::HookRegistry hookRegistry{ idGenerator };

        support::SupportCodeLibrary supportCodeLibrary{
            .hookRegistry = hookRegistry,
            .stepRegistry = stepRegistry,
            .parameterRegistry = parameterRegistry,
            .undefinedParameters = undefinedParameters,
        };

        query::Query query{ broadcaster };

        const auto formatOptionsJson = formatOptions.empty() ? nlohmann::json::object() : nlohmann::json::parse(formatOptions);
        const auto activeFormatters = formatters.EnableFormatters(format, formatOptionsJson, supportCodeLibrary, query);

        const auto pickleSources = CollectPickles(options.sources, idGenerator, broadcaster);
        const auto orderedPickles = OrderPickles(options.sources, pickleSources | std::views::filter(FilterByTagExpression(options.sources)));

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        const auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, orderedPickles, supportCodeLibrary, idGenerator, programContext);
        return runtime->Run();
    }
}
