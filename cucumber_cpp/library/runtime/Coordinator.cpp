#include "cucumber_cpp/library/runtime/Coordinator.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/gherkin/exceptions.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/gherkin/pickle_compiler.hpp"
#include "cucumber/gherkin/utils.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/parse_error.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/source.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber/messages/suggestion.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_run_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/timestamp.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <algorithm>
#include <any>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    Coordinator::Coordinator(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        std::span<const support::PickleSource> sourcedPickles,
        std::unique_ptr<support::RuntimeAdapter>&& runtimeAdapter,
        support::SupportCodeLibrary supportCodeLibrary)
        : testRunStartedId{ testRunStartedId }
        , broadcaster{ broadcaster }
        , idGenerator{ idGenerator }
        , sourcedPickles{ sourcedPickles }
        , runtimeAdapter{ std::move(runtimeAdapter) }
        , supportCodeLibrary{ supportCodeLibrary }
    {}

    bool Coordinator::Run()
    {
        broadcaster.BroadcastEvent({ .test_run_started = cucumber::messages::test_run_started{
                                         .timestamp = support::TimestampNow(),
                                         .id = std::string{ testRunStartedId },
                                     } });

        const auto assembledTestCases = assemble::AssembleTestSuites(supportCodeLibrary, testRunStartedId, broadcaster, sourcedPickles, idGenerator);
        const auto success = runtimeAdapter->Run(assembledTestCases);

        broadcaster.BroadcastEvent({ .test_run_finished = cucumber::messages::test_run_finished{
                                         .success = success,
                                         .timestamp = support::TimestampNow(),
                                         .test_run_started_id = std::string{ testRunStartedId },
                                     } });

        return success;
    }
}
