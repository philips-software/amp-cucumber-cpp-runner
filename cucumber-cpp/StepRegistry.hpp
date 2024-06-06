#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber-cpp/Body.hpp"
#include "cucumber-cpp/Context.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <regex>
#include <source_location>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    enum struct StepType
    {
        given,
        when,
        then,
        any
    };

    struct TableValue
    {
        template<class T>
        T As(std::source_location sourceLocation = std::source_location::current()) const;

        std::string value;
    };

    using Table = std::vector<std::vector<TableValue>>;

    struct Step
    {
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

        Step(Context& context, const Table& table);
        virtual ~Step() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }

    protected:
        void Given(const std::string& step);
        void When(const std::string& step);
        void Then(const std::string& step);

        [[noreturn]] void Pending(const std::string& message, std::source_location current = std::source_location::current()) const noexcept(false);

    private:
        void Any(StepType type, const std::string& step);

    protected:
        Context& context;
        const Table& table;
    };

    struct RegexMatch
    {
        RegexMatch(const std::regex& regex, const std::string& expression);

        [[nodiscard]] bool Matched() const;
        [[nodiscard]] std::vector<std::string> Matches() const;

    private:
        bool matched;
        std::vector<std::string> matches;
    };

    struct StepRegex
    {
        explicit StepRegex(const std::string& string);

        [[nodiscard]] std::unique_ptr<RegexMatch> Match(const std::string& expression) const;
        [[nodiscard]] std::string String() const;

    private:
        std::string string;
        std::regex regex;
    };

    struct StepMatch
    {
        std::unique_ptr<Body> (&factory)(Context& context, const Table& table);
        std::vector<std::string> matches{};
        std::string stepRegexStr{};
    };

    struct StepRegistryBase
    {
        struct StepNotFoundError : std::exception
        {
            using std::exception::exception;
        };

        struct AmbiguousStepError : std::exception
        {
            explicit AmbiguousStepError(std::vector<StepMatch>&& matches)
                : matches{ std::move(matches) }
            {}

            std::vector<StepMatch> matches;
        };

        struct Entry
        {
            StepType type{};
            StepRegex regex;
            std::unique_ptr<Body> (&factory)(Context& context, const Table& table);

            std::uint32_t used{ 0 };
        };

        struct EntryView
        {
            const StepRegex& stepRegex;
            const std::uint32_t& used;
        };

        [[nodiscard]] StepMatch Query(StepType stepType, const std::string& expression);

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(StepType stepType) const;

        std::vector<EntryView> List() const;

    protected:
        template<class T>
        std::size_t
        Register(const std::string& matcher, StepType stepType);

    private:
        template<class T>
        static std::unique_ptr<Body> Construct(Context& context, const Table& table);

        std::vector<Entry> registry;
    };

    struct StepRegistry : StepRegistryBase
    {
    private:
        StepRegistry() = default;

    public:
        static StepRegistry& Instance();

        template<class T>
        static std::size_t Register(const std::string& matcher, StepType stepType);
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    T TableValue::As(std::source_location sourceLocation) const
    {
        return StringTo<T>(value, sourceLocation);
    }

    template<class T>
    std::size_t StepRegistryBase::Register(const std::string& matcher, StepType stepType)
    {
        registry.emplace_back(stepType, StepRegex{ matcher }, Construct<T>);
        return registry.size();
    }

    template<class T>
    std::unique_ptr<Body> StepRegistryBase::Construct(Context& context, const Table& table)
    {
        return std::make_unique<T>(context, table);
    }

    template<class T>
    std::size_t StepRegistry::Register(const std::string& matcher, StepType stepType)
    {
        return Instance().StepRegistryBase::Register<T>(matcher, stepType);
    }
}

#endif
