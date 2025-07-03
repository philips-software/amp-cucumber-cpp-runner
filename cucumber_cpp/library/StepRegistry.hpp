#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    template<class T>
    std::unique_ptr<Body> StepBodyFactory(Context& context, const engine::Table& table)
    {
        return std::make_unique<T>(context, table);
    }

    struct StepMatch
    {
        StepMatch(std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table), std::variant<std::vector<std::string>, std::vector<std::any>> matches, std::string_view stepRegexStr)
            : factory(factory)
            , matches(std::move(matches))
            , stepRegexStr(stepRegexStr)
        {}

        std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table);
        std::variant<std::vector<std::string>, std::vector<std::any>> matches{};
        std::string_view stepRegexStr{};
    };

    struct StepRegistry
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
            Entry(engine::StepType type, cucumber_expression::Matcher regex, std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table))
                : type(type)
                , regex(std::move(regex))
                , factory(factory)
            {}

            engine::StepType type{};
            cucumber_expression::Matcher regex;
            std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table);

            std::uint32_t used{ 0 };
        };

        struct EntryView
        {
            EntryView(const cucumber_expression::Matcher& stepRegex, const std::uint32_t& used)
                : stepRegex(stepRegex)
                , used(used)
            {}

            const cucumber_expression::Matcher& stepRegex;
            const std::uint32_t& used;
        };

        explicit StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry);

        [[nodiscard]] StepMatch Query(const std::string& expression);

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] std::vector<EntryView> List() const;

    private:
        void Register(const std::string& matcher, engine::StepType stepType, std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table));

        std::vector<Entry> registry;
        cucumber_expression::ParameterRegistry& parameterRegistry;
    };

    struct StepStringRegistration
    {
    private:
        StepStringRegistration() = default;

    public:
        static StepStringRegistration& Instance();

        struct Entry
        {
            Entry(engine::StepType type, std::string regex, std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table))
                : type(type)
                , regex(std::move(regex))
                , factory(factory)
            {}

            engine::StepType type{};
            std::string regex;
            std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table);
        };

        template<class T>
        static std::size_t Register(const std::string& matcher, engine::StepType stepType);

        std::span<Entry> GetEntries();
        [[nodiscard]] std::span<const Entry> GetEntries() const;

    private:
        std::vector<Entry> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t StepStringRegistration::Register(const std::string& matcher, engine::StepType stepType)
    {
        Instance().registry.emplace_back(stepType, matcher, StepBodyFactory<T>);

        return Instance().registry.size();
    }
}

#endif
