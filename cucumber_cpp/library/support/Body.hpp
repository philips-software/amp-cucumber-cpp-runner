#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include <any>
#include <concepts>
#include <exception>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
    using ExecuteArgs = std::variant<std::vector<std::string>, std::vector<std::any>>;

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

    struct Body
    {
        virtual ~Body() = default;

        cucumber::messages::test_step_result ExecuteAndCatchExceptions(const cucumber::messages::step_match_arguments_list& args = {});

    protected:
        virtual void Execute(const cucumber::messages::step_match_arguments_list& args) = 0;
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
