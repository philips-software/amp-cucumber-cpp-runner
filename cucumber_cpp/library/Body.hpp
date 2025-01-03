#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include <string>
#include <vector>

namespace cucumber_cpp::library
{
    struct Body
    {
        virtual ~Body() = default;

        virtual void Execute(const std::vector<std::string>& args = {}) = 0;
    };

    template<class T>
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
