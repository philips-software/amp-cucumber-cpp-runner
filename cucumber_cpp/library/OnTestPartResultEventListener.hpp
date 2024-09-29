#ifndef CUCUMBER_CPP_ONTESTPARTRESULTEVENTLISTENER_HPP
#define CUCUMBER_CPP_ONTESTPARTRESULTEVENTLISTENER_HPP

#include <gtest/gtest.h>

namespace cucumber_cpp
{
    struct OnTestPartResultEventListener : testing::EmptyTestEventListener
    {
        OnTestPartResultEventListener();
        ~OnTestPartResultEventListener() override;

        OnTestPartResultEventListener(const OnTestPartResultEventListener&) = delete;
        OnTestPartResultEventListener& operator=(const OnTestPartResultEventListener&) = delete;

        OnTestPartResultEventListener(OnTestPartResultEventListener&&) = delete;
        OnTestPartResultEventListener& operator=(OnTestPartResultEventListener&&) = delete;

    private:
        testing::TestEventListeners& listeners{ testing::UnitTest::GetInstance()->listeners() };
    };
}

#endif
