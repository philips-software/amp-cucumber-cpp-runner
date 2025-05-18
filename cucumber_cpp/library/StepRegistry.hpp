#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
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

        [[nodiscard]] StepMatch Query(engine::StepType stepType, const std::string& expression);

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(engine::StepType stepType) const;

        [[nodiscard]] std::vector<EntryView> List() const;

    protected:
        template<class T>
        std::size_t
        Register(const std::string& matcher, engine::StepType stepType);

    private:
        template<class T>
        static std::unique_ptr<Body> Construct(Context& context, const engine::Table& table);

        std::vector<Entry> registry;
        cucumber_expression::ParameterRegistry parameterRegistry;
    };

    struct StepRegistry : StepRegistryBase
    {
    private:
        StepRegistry() = default;

    public:
        static StepRegistry& Instance();

        template<class T>
        static std::size_t Register(const std::string& matcher, engine::StepType stepType);
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t StepRegistryBase::Register(const std::string& matcher, engine::StepType stepType)
    {
        if (matcher.starts_with('^') || matcher.ends_with('$'))
            registry.emplace_back(stepType, cucumber_expression::Matcher{ std::in_place_type<cucumber_expression::RegularExpression>, matcher }, Construct<T>);
        else
            registry.emplace_back(stepType, cucumber_expression::Matcher{ std::in_place_type<cucumber_expression::Expression>, matcher, parameterRegistry }, Construct<T>);
        return registry.size();
    }

    template<class T>
    std::unique_ptr<Body> StepRegistryBase::Construct(Context& context, const engine::Table& table)
    {
        return std::make_unique<T>(context, table);
    }

    template<class T>
    std::size_t StepRegistry::Register(const std::string& matcher, engine::StepType stepType)
    {
        return Instance().StepRegistryBase::Register<T>(matcher, stepType);
    }
}

#endif
