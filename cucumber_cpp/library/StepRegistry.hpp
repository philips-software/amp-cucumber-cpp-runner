#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library
{
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
        StepMatch(std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table), const std::vector<std::string>& matches, const std::string& stepRegexStr)
            : factory(factory)
            , matches(matches)
            , stepRegexStr(stepRegexStr)
        {}

        std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table);
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
            Entry(engine::StepType type, StepRegex regex, std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table))
                : type(type)
                , regex(std::move(regex))
                , factory(factory)
            {}

            engine::StepType type{};
            StepRegex regex;
            std::unique_ptr<Body> (&factory)(Context& context, const engine::Table& table);

            std::uint32_t used{ 0 };
        };

        struct EntryView
        {
            EntryView(const StepRegex& stepRegex, const std::uint32_t& used)
                : stepRegex(stepRegex)
                , used(used)
            {}

            const StepRegex& stepRegex;
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
        registry.emplace_back(stepType, StepRegex{ matcher }, Construct<T>);
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
