#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/formatter/PrettyPrinter.hpp"
#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/runtime/MakeRuntime.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <gtest/gtest.h>
#include <list>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library::api
{
    namespace
    {
        void EmitParameters(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
        {
            for (const auto& [name, parameter] : supportCodeLibrary.parameterRegistry.GetParameters())
            {
                if (parameter.isBuiltin)
                    continue;

                broadcaster.BroadcastEvent({
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

        void EmitUndefinedParameters(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& parameter : supportCodeLibrary.undefinedParameters.definitions)
                broadcaster.BroadcastEvent({ .undefined_parameter_type = parameter });
        }

        void EmitStepDefinitions(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& stepDefinition : supportCodeLibrary.stepRegistry.StepDefinitions())
            {
                broadcaster.BroadcastEvent({ .step_definition = cucumber::messages::step_definition{
                                                 .id = stepDefinition.id,
                                                 .pattern = cucumber::messages::step_definition_pattern{
                                                     .source = stepDefinition.pattern,
                                                     .type = stepDefinition.patternType,
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

        void EmitTestCaseHooks(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::before);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::after);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });
        }

        void EmitTestRunHooks(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::beforeAll);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::afterAll);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });
        }

        void EmitSupportCodeMessages(support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
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
    }

    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string>& format, const std::string& formatOptions)
    {
        cucumber::gherkin::id_generator_ptr idGenerator = std::make_shared<cucumber::gherkin::id_generator>();

        support::UndefinedParameters undefinedParameters;
        StepRegistry stepRegistry{ parameterRegistry, undefinedParameters, idGenerator };
        HookRegistry hookRegistry{ idGenerator };

        support::SupportCodeLibrary supportCodeLibrary{
            .hookRegistry = hookRegistry,
            .stepRegistry = stepRegistry,
            .parameterRegistry = parameterRegistry,
            .undefinedParameters = undefinedParameters,
        };

        formatter::helper::EventDataCollector eventDataCollector{ broadcaster };
        Query query{ broadcaster };

        const auto activeFormatters = formatters.EnableFormatters(format, formatOptions, supportCodeLibrary, query, eventDataCollector);

        const auto pickleSources = CollectPickles(options.sources, idGenerator, broadcaster);

        const auto pickleFilter = [&options](const support::PickleSource& pickle)
        {
            return options.sources.tagExpression->Evaluate(pickle.pickle->tags);
        };
        auto filteredPicklesView = pickleSources | std::views::filter(pickleFilter);

        const auto createOrderedPickleList = [](auto ordered) -> std::list<support::PickleSource>
        {
            return { ordered.begin(), ordered.end() };
        };
        const auto orderPickles = [&](auto pickles) -> std::list<support::PickleSource>
        {
            if (options.sources.ordering == support::RunOptions::Ordering::defined)
                return createOrderedPickleList(pickles);
            else
                return createOrderedPickleList(pickles | std::views::reverse);
        };
        std::list<support::PickleSource> orderedPickles = orderPickles(filteredPicklesView);

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, orderedPickles, supportCodeLibrary, idGenerator, programContext);

        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        auto* defaultEventListener = listeners.Release(listeners.default_result_printer());

        auto result = runtime->Run();

        listeners.Append(defaultEventListener);

        return result;
    }
}
