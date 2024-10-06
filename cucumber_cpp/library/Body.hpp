#ifndef CUCUMBER_CPP_BODY_HPP
#define CUCUMBER_CPP_BODY_HPP

#include "cucumber_cpp/library/InternalError.hpp"
#include <source_location>
#include <sstream>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    template<class To>
    inline To StringTo(const std::string& s, std::source_location sourceLocation = std::source_location::current())
    {
        std::istringstream stream{ s };
        To to;
        stream >> to;
        if (stream.fail())
            throw InternalError{ "Cannnot convert parameter \"" + s + "\"", sourceLocation };
        return to;
    }

    template<>
    inline std::string StringTo<std::string>(const std::string& s, std::source_location /*sourceLocation*/)
    {
        return s;
    }

    struct Body
    {
        virtual ~Body() = default;

        virtual void Execute(const std::vector<std::string>& args = {}) = 0;
    };

    template<class T>
    struct SetUpTearDownWrapper
    {
        SetUpTearDownWrapper(T& t)
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
