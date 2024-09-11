#ifndef CUCUMBER_CPP_ONTESTPARTRESULTEVENTLISTENER_HPP
#define CUCUMBER_CPP_ONTESTPARTRESULTEVENTLISTENER_HPP

#include <gtest/gtest.h>

namespace cucumber_cpp
{
    struct OnTestPartResultEventListener : testing::EmptyTestEventListener
    {
        OnTestPartResultEventListener();
        ~OnTestPartResultEventListener() override;

    private:
        testing::TestEventListeners& listeners{ testing::UnitTest::GetInstance()->listeners() };
    };
}

#endif
