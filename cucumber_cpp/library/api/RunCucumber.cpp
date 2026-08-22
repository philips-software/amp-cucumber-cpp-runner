#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/ParameterType.hpp"
#include "cucumber/messages/ParseError.hpp"
#include "cucumber/messages/PickleTag.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/StepDefinitionPattern.hpp"
#include "cucumber/messages/StepDefinitionPatternType.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/api/Gherkin.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
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
#include "cucumber_cpp/library/util/MakeShared.hpp"
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::api
{
    namespace
    {
        void EmitParameters(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster, const cucumber::gherkin::IdGeneratorPtr& idGenerator)
        {
            for (const auto& [name, parameter] : supportCodeLibrary.parameterRegistry.GetParameters())
            {
                if (parameter.isBuiltin)
                    continue;

                broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::ParameterType{
                    .name = parameter.name,
                    .regularExpressions = parameter.regex,
                    .useForSnippets = parameter.useForSnippets,
                    .id = idGenerator->NextId(),
                    .sourceReference = util::MakeShared(cucumber::messages::SourceReference{
                        .uri = parameter.location.file_name(),
                        .location = util::MakeShared(cucumber::messages::Location{
                            .line = parameter.location.line(),
                        }),
                    }),
                }));
            }
        }

        void EmitUndefinedParameters(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster)
        {
            for (const auto& parameter : supportCodeLibrary.undefinedParameters.definitions)
                broadcaster.BroadcastEvent(util::MakeShared(parameter));
        }

        void EmitStepDefinitions(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster)
        {
            for (const auto& stepDefinition : supportCodeLibrary.stepRegistry.StepDefinitions())
            {
                broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::StepDefinition{
                    .id = stepDefinition.id,
                    .pattern = util::MakeShared(cucumber::messages::StepDefinitionPattern{
                        .source = stepDefinition.pattern,
                        .type = stepDefinition.patternType == support::ExpressionPatternType::cucumberExpression ? cucumber::messages::StepDefinitionPatternType::CUCUMBER_EXPRESSION : cucumber::messages::StepDefinitionPatternType::REGULAR_EXPRESSION,
                    }),
                    .sourceReference = util::MakeShared(cucumber::messages::SourceReference{
                        .uri = stepDefinition.uri.string(),
                        .location = util::MakeShared(cucumber::messages::Location{
                            .line = stepDefinition.line,
                        }),
                    }),
                }));
            }
        }

        void EmitTestCaseHooks(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::before);

            for (const auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent(util::MakeShared(util::TransformHookData(hook)));

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::after);

            for (const auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent(util::MakeShared(util::TransformHookData(hook)));
        }

        void EmitTestRunHooks(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster)
        {
            auto beforeAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::beforeAll);

            for (const auto& hook : beforeAllHooks)
                broadcaster.BroadcastEvent(util::MakeShared(util::TransformHookData(hook)));

            auto afterAllHooks = supportCodeLibrary.hookRegistry.HooksByType(util::HookType::afterAll);

            for (const auto& hook : afterAllHooks)
                broadcaster.BroadcastEvent(util::MakeShared(util::TransformHookData(hook)));
        }

        void EmitSupportCodeMessages(const support::SupportCodeLibrary& supportCodeLibrary, const util::Broadcaster& broadcaster, const cucumber::gherkin::IdGeneratorPtr& idGenerator)
        {
            // Phase 1: Load parameters (must be first, steps reference parameter types)
            plugin::ParameterLoader::Load(support::DefinitionRegistration::Instance(), supportCodeLibrary.parameterRegistry);
            EmitParameters(supportCodeLibrary, broadcaster, idGenerator);

            support::DefinitionRegistration::Instance().LoadIds(idGenerator);

            // Phase 2: Load steps (can now resolve parameter type expressions)
            plugin::StepLoader::Load(supportCodeLibrary.stepRegistry);

            EmitUndefinedParameters(supportCodeLibrary, broadcaster);
            EmitStepDefinitions(supportCodeLibrary, broadcaster);

            // Phase 3: Load hooks (last, no ordering dependency)
            plugin::HookLoader::Load(supportCodeLibrary.hookRegistry);
            EmitTestCaseHooks(supportCodeLibrary, broadcaster);
            EmitTestRunHooks(supportCodeLibrary, broadcaster);
        }

        const std::string& TransformPickleTagName(const std::shared_ptr<cucumber::messages::PickleTag>& tag)
        {
            return tag->name;
        }

        std::set<std::string, std::less<>> PickleTagsToSet(const std::vector<std::shared_ptr<cucumber::messages::PickleTag>>& tags)
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
                return createOrderedPickleList(std::move(pickles));
            else
                return createOrderedPickleList(std::move(pickles) | std::views::reverse);
        };

        struct ParseErrorListener : util::Listener
        {
            explicit ParseErrorListener(util::Broadcaster& broadcaster)
                : Listener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
                    {
                        OnEvent(envelope);
                    } }
            {}

            void OnEvent(const cucumber::messages::Envelope& envelope)
            {
                if (envelope.parseError)
                    parseErrors.push_back(*envelope.parseError.value());
            }

            [[nodiscard]] const std::vector<cucumber::messages::ParseError>& GetParseErrors() const
            {
                return parseErrors;
            }

        private:
            std::vector<cucumber::messages::ParseError> parseErrors;
        };
    }

    bool RunCucumber(const support::RunOptions& options, cucumber_expression::ParameterRegistry& parameterRegistry, Context& programContext, util::Broadcaster& broadcaster, Formatters& formatters, const std::set<std::string, std::less<>>& format, const std::string& formatOptions)
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

        ParseErrorListener parseErrorListener{ broadcaster };
        auto pickleSources = CollectPickles(options.sources, idGenerator, broadcaster);

        if (const auto& parseErrors = parseErrorListener.GetParseErrors(); !parseErrors.empty())
        {
            for (const auto& parseError : parseErrors)
            {
                const auto uri = parseError.source->uri.value_or("unknown source");
                const auto line = parseError.source->location.has_value() ? fmt::format(":{}", parseError.source->location.value()->line) : "";
                const auto column = parseError.source->location.has_value() && parseError.source->location.value()->column.has_value() ? fmt::format(":{}", parseError.source->location.value()->column.value()) : "";

                const auto messageStart = parseError.message.find(": ");
                const auto message = messageStart != std::string::npos ? parseError.message.substr(messageStart + 2) : parseError.message;

                fmt::println(std::cerr, "Parse error in: \"{}{}{}\" {}", uri, line, column, message);
            }

            return false;
        }

        const auto orderedPickles = OrderPickles(options.sources, pickleSources | std::views::filter(FilterByTagExpression(options.sources)));

        EmitSupportCodeMessages(supportCodeLibrary, broadcaster, idGenerator);

        const auto runtime = runtime::MakeRuntime(options.runtime, broadcaster, orderedPickles, supportCodeLibrary, idGenerator, programContext);
        return runtime->Run();
    }
}
