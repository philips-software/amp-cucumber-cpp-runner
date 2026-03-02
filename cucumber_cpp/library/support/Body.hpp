#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::support
{
    struct Argument
    {
        std::string converterName;
        cucumber_expression::ConvertFunctionArg converterArgs;
    };

    using ExecuteArgs = std::vector<Argument>;

    struct FatalError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct StepSkipped : std::exception
    {
        StepSkipped(std::string message, std::source_location sourceLocation)
            : message{ std::move(message) }
            , sourceLocation{ sourceLocation }
        {
        }

        std::string message;
        std::source_location sourceLocation;
    };

    struct StepPending : std::exception
    {
        StepPending(std::string message, std::source_location sourceLocation)
            : message{ std::move(message) }
            , sourceLocation{ sourceLocation }
        {
        }

        std::string message;
        std::source_location sourceLocation;
    };

    enum class TestStepResultStatus : std::uint8_t
    {
        UNKNOWN,
        PASSED,
        SKIPPED,
        PENDING,
        UNDEFINED,
        AMBIGUOUS,
        FAILED
    };

    struct TestException
    {
        std::string type;
        std::optional<std::string> message;
    };

    struct TestDuration
    {
        std::size_t seconds{};
        std::size_t nanos{};
    };

    struct TestStepResult
    {
        TestDuration duration{};

        std::optional<std::string> message;

        TestStepResultStatus status{};

        std::optional<TestException> exception;
    };

    struct Body
    {
        virtual ~Body() = default;

        TestStepResult ExecuteAndCatchExceptions(const ExecuteArgs& args = {});

    protected:
        virtual void Execute(const ExecuteArgs& args) = 0;
    };

    template<typename T>
    concept HasSetUpTearDown =
        requires(T t) {
            { t.SetUp() } -> std::convertible_to<void>;
            { t.TearDown() } -> std::convertible_to<void>;
        };

    template<HasSetUpTearDown T>
    struct SetUpTearDownWrapper
    {
        explicit SetUpTearDownWrapper(T& t)
            : t{ t }
        {
            t.SetUp();
        }

        SetUpTearDownWrapper(const SetUpTearDownWrapper&) = delete;
        SetUpTearDownWrapper(SetUpTearDownWrapper&&) = delete;

        ~SetUpTearDownWrapper()
        {
            t.TearDown();
        }

    private:
        T& t;
    };
}

#endif
