#ifndef CUCUMBER_CPP_STEPS_HPP
#define CUCUMBER_CPP_STEPS_HPP

#include "cucumber-cpp/Context.hpp"
#include "nlohmann/json.hpp"
#include <map>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp
{

    template<class To>
    inline To StringTo(const std::string& s)
    {
        std::istringstream stream{ s };
        To to;
        stream >> to;
        if (stream.fail())
        {
            throw std::invalid_argument("Cannot convert parameter");
        }
        return to;
    }

    template<>
    inline std::string StringTo<std::string>(const std::string& s)
    {
        return s;
    }

    struct StepNotFoundException : std::out_of_range
    {
        using std::out_of_range::out_of_range;
    };

    enum class StepType
    {
        given,
        when,
        then,
        any
    };

    struct RegexMatch
    {
        RegexMatch(const std::regex regex, const std::string& expression);

        bool Matched() const;
        std::vector<std::string> Matches() const;

    private:
        bool matched;
        std::vector<std::string> matches;
    };

    struct StepRegex
    {
        explicit StepRegex(const std::string& string);

        std::shared_ptr<RegexMatch> Match(const std::string& expression) const;
        std::string String() const;

    private:
        std::string string;
        std::regex regex;
    };

    struct StepImplementation;

    struct StepMatch
    {
        std::shared_ptr<StepImplementation> step;
        std::shared_ptr<RegexMatch> regexMatch;
    };

    struct StepImplementation
    {
        StepImplementation();
        virtual ~StepImplementation() = default;

        std::size_t Id() const;
        void SetRegex(const std::string& str);
        StepRegex& Regex() const;

        virtual const std::string& GetText() const = 0;
        virtual StepType GetType() const = 0;
        virtual void Run(Context& context, const nlohmann::json& parameters, const nlohmann::json& table) = 0;

    private:
        static std::size_t counter;
        std::size_t id;
        std::unique_ptr<StepRegex> regex;
    };

    struct StepRepository
    {
        explicit StepRepository();

        StepMatch Get(StepType stepType, std::string text) const noexcept(false);

        void RegisterStepImplementation(std::shared_ptr<StepImplementation> step);

        std::size_t Size() const;
        std::size_t Size(StepType stepType) const;

    private:
        StepMatch GetByType(StepType stepType, std::string text) const;

        using StepsVector = std::vector<std::shared_ptr<StepImplementation>>;
        static std::map<StepType, StepsVector> stepRepository;
    };

    struct StepPreRegistration
    {
        static bool Register(std::shared_ptr<StepImplementation> step)
        {
            Instance().registration.push_back(std::move(step));
            return true;
        }

        static void RegisterAll(StepRepository& stepRepository)
        {
            auto& registration = Instance().registration;

            for (auto i = 0; i < registration.size(); ++i)
            {
                stepRepository.RegisterStepImplementation(registration[i]);
            }

            registration.clear();
        }

    private:
        static StepPreRegistration& Instance()
        {
            static StepPreRegistration instance;
            return instance;
        }

        std::vector<std::shared_ptr<StepImplementation>> registration;
    };

    struct StepFixture : StepImplementation
    {
    protected:
        StepFixture(std::string text, StepType stepType)
            : text{ text }
            , stepType{ stepType }
        {
        }

        const std::string& GetText() const override
        {
            return text;
        }

        StepType GetType() const override
        {
            return stepType;
        }

        template<class T, class... Args, std::size_t... I>
        void InvokeWithArgImpl(T* t, const nlohmann::json& json, void (T::*ptr)(Args...), std::index_sequence<I...>)
        {
            (t->*ptr)(StringTo<Args>(json[I])...);
        }

        template<class T, class... Args>
        void InvokeWithArg(T* t, const nlohmann::json& json, void (T::*ptr)(Args...))
        {
            InvokeWithArgImpl(t, json, ptr, std::make_index_sequence<sizeof...(Args)>{});
        }

    private:
        std::string text;
        StepType stepType;
    };
}

#define CONCAT_(lhs, rhs) lhs##rhs
#define CONCAT(lhs, rhs) CONCAT_(lhs, rhs)

#define STEP_MATCHER(matcher, ...) matcher
#define STEP_ARGS(matcher, args, ...) args

#define STEP_(str, type, args)                                                                                                                                       \
    namespace                                                                                                                                                        \
    {                                                                                                                                                                \
        struct CONCAT(StepImpl_, __LINE__)                                                                                                                           \
            : cucumber_cpp::StepFixture                                                                                                                              \
        {                                                                                                                                                            \
            CONCAT(StepImpl_, __LINE__)()                                                                                                                            \
                : cucumber_cpp::StepFixture{ str, type }                                                                                                             \
            {                                                                                                                                                        \
            }                                                                                                                                                        \
                                                                                                                                                                     \
            void Run(cucumber_cpp::Context& context, const nlohmann::json& parameters, const nlohmann::json& table) override                                         \
            {                                                                                                                                                        \
                this->context = &context;                                                                                                                            \
                this->table = &table;                                                                                                                                \
                InvokeWithArg(this, parameters, &CONCAT(StepImpl_, __LINE__)::RunWithArgs);                                                                          \
            }                                                                                                                                                        \
                                                                                                                                                                     \
        private:                                                                                                                                                     \
            void RunWithArgs args;                                                                                                                                   \
            cucumber_cpp::Context* context;                                                                                                                          \
            const nlohmann::json* table;                                                                                                                             \
        };                                                                                                                                                           \
        const bool CONCAT(CONCAT(StepImpl_, __LINE__),IsRegistered){ cucumber_cpp::StepPreRegistration::Register(std::make_shared<CONCAT(StepImpl_, __LINE__)>()) }; \
    }                                                                                                                                                                \
    inline void CONCAT(StepImpl_, __LINE__)::RunWithArgs args

#define GIVEN(...)                     \
    STEP_(                             \
        STEP_MATCHER(__VA_ARGS__, ()), \
        cucumber_cpp::StepType::given, \
        STEP_ARGS(__VA_ARGS__, (), ()))

#define WHEN(...)                      \
    STEP_(                             \
        STEP_MATCHER(__VA_ARGS__, ()), \
        cucumber_cpp::StepType::when,  \
        STEP_ARGS(__VA_ARGS__, (), ()))

#define THEN(...)                      \
    STEP_(                             \
        STEP_MATCHER(__VA_ARGS__, ()), \
        cucumber_cpp::StepType::then,  \
        STEP_ARGS(__VA_ARGS__, (), ()))

#define STEP(...)                      \
    STEP_(                             \
        STEP_MATCHER(__VA_ARGS__, ()), \
        cucumber_cpp::StepType::any,   \
        STEP_ARGS(__VA_ARGS__, (), ()))

#endif
