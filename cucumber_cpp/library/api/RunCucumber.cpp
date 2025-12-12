#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/formatter/PrettyPrinter.hpp"
#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/runtime/MakeRuntime.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <ranges>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        void EmitParameters(support::SupportCodeLibrary supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
        {
            for (const auto& [name, parameter] : supportCodeLibrary.parameterRegistry.GetParameters())
            {
                if (parameter.isBuiltin)
                    continue;

                broadcaster.BroadcastEvent({ .parameter_type = cucumber::messages::parameter_type{
                                                 .name = parameter.name,
                                                 .regular_expressions = parameter.regex,
                                                 .id = idGenerator->next_id(),
                                             } });
            }
        }

        void EmitStepDefinitions(support::SupportCodeLibrary supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
        {
            supportCodeLibrary.stepRegistry.LoadSteps();

            for (const auto& [name, stepDefinition] : supportCodeLibrary.stepRegistry.StepDefinitions())
            {
                broadcaster.BroadcastEvent({ .step_definition = cucumber::messages::step_definition{
                                                 .id = stepDefinition.id,
                                                 .pattern = cucumber::messages::step_definition_pattern{
                                                     .source = stepDefinition.pattern,
                                                     .type = stepDefinition.patternType,
                                                 },
                                                 .source_reference = {
                                                     .uri = stepDefinition.uri,
                                                     .location = cucumber::messages::location{
                                                         .line = stepDefinition.line,
                                                     },
                                                 },
                                             } });
            }
        }

        void EmitSupportCodeMessages(support::SupportCodeLibrary supportCodeLibrary, util::Broadcaster& broadcaster, cucumber::gherkin::id_generator_ptr idGenerator)
        {
            EmitParameters(supportCodeLibrary, broadcaster, idGenerator);
            // undefined parameters
            EmitStepDefinitions(supportCodeLibrary, broadcaster, idGenerator);
            // test case hooks
            // test run hooks
        }
    }

    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster)
    {
        cucumber::gherkin::id_generator_ptr idGenerator = std::make_shared<cucumber::gherkin::id_generator>();

        StepRegistry stepRegistry{ parameterRegistry, idGenerator };
        HookRegistry hookRegistry{};

        support::SupportCodeLibrary supportCodeLibrary{ .hookRegistry = hookRegistry, .stepRegistry = stepRegistry, .parameterRegistry = parameterRegistry };

        formatter::helper::EventDataCollector eventDataCollector{ broadcaster };
        formatter::PrettyPrinter prettyPrinter{ supportCodeLibrary, broadcaster, eventDataCollector };
        formatter::SummaryFormatter summaryFormatter{ supportCodeLibrary, broadcaster, eventDataCollector };

        const auto pickleSources = CollectPickles(options.sources, idGenerator, broadcaster);

        const auto tagExpression = cucumber_cpp::library::tag_expression::Parse(options.sources.tagExpression);
        const auto pickleFilter = [&tagExpression](const support::PickleSource& pickle)
        {
            return tagExpression->Evaluate(pickle.pickle->tags);
        };
        auto filteredPicklesView = pickleSources | std::views::filter(pickleFilter);
        std::vector<support::PickleSource> filteredPickles(filteredPicklesView.begin(), filteredPicklesView.end());

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, filteredPickles, supportCodeLibrary, idGenerator, programContext);

        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        auto* defaultEventListener = listeners.Release(listeners.default_result_printer());

        auto result = runtime->Run();

        listeners.Append(defaultEventListener);

        return result;
    }
}
