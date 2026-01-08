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
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/runtime/MakeRuntime.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "nlohmann/json_fwd.hpp"
#include <csignal>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
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
        void EmitParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
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

        void EmitUndefinedParameters(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            for (const auto& parameter : supportCodeLibrary.undefinedParameters.definitions)
                broadcaster.BroadcastEvent({ .undefined_parameter_type = parameter });
        }

        void EmitStepDefinitions(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
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

        void EmitTestCaseHooks(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::before);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::after);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });
        }

        void EmitTestRunHooks(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::beforeAll);

            for (auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(HookType::afterAll);

            for (auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent({ .hook = std::move(hook) });
        }

        void EmitSupportCodeMessages(const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
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

        auto FilterByTagExpression(const support::RunOptions::Sources& sources)
        {
            return [&sources](const support::PickleSource& pickle)
            {
                return sources.tagExpression->Evaluate(pickle.pickle->tags);
            };
        }

        std::list<support::PickleSource> OrderPickles(const support::RunOptions::Sources& sources, auto pickles)
        {
            const auto createOrderedPickleList = [](auto ordered) -> std::list<support::PickleSource>
            {
                return { ordered.begin(), ordered.end() };
            };

            if (sources.ordering == support::RunOptions::Ordering::defined)
                return createOrderedPickleList(pickles);
            else
                return createOrderedPickleList(pickles | std::views::reverse);
        };

        struct RemoveDefaultEventListener
        {
            ~RemoveDefaultEventListener()
            {
                listeners.Append(defaultEventListener);
            }

        private:
            testing::TestEventListeners& listeners{ testing::UnitTest::GetInstance()->listeners() };
            testing::TestEventListener* defaultEventListener{ listeners.Release(listeners.default_result_printer()) };
        };

        void signal_handler(int signal)
        {
            if (signal == SIGABRT)
                std::cerr << "SIGABRT received\n";
            else
                std::cerr << "Unexpected signal " << signal << " received\n";
            std::_Exit(EXIT_FAILURE);
        }

        struct OverrideAbortSignalHandler
        {
            ~OverrideAbortSignalHandler()
            {
                std::signal(SIGABRT, original);
            }

            using signal_handler_t = void (*)(int);
            signal_handler_t original{ std::signal(SIGABRT, signal_handler) };
        };
    }

    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string, std::less<>>& format, const std::string& formatOptions)
    {
        RemoveDefaultEventListener removeDefaultListenerExceptionSafe;
        OverrideAbortSignalHandler overrideSignalHandler;

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

        const auto formatOptionsJson = formatOptions.empty() ? nlohmann::json::object() : nlohmann::json::parse(formatOptions);
        const auto activeFormatters = formatters.EnableFormatters(format, formatOptionsJson, supportCodeLibrary, query, eventDataCollector);

        const auto pickleSources = CollectPickles(options.sources, idGenerator, broadcaster);
        const auto orderedPickles = OrderPickles(options.sources, pickleSources | std::views::filter(FilterByTagExpression(options.sources)));

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        const auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, orderedPickles, supportCodeLibrary, idGenerator, programContext);
        return runtime->Run();
    }
}
