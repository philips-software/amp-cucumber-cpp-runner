#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include <any>
#include <concepts>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    struct Body
    {
        virtual ~Body() = default;

        virtual void Execute(const std::variant<std::vector<std::string>, std::vector<std::any>>& args = {}) = 0;
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
