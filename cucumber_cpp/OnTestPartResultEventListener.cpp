
#include "cucumber_cpp/OnTestPartResultEventListener.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace cucumber_cpp
{
    OnTestPartResultEventListener::OnTestPartResultEventListener()
    {
        if (auto* defaultPrinter = listeners.default_result_printer(); defaultPrinter != nullptr)
            std::unique_ptr<TestEventListener>{ listeners.Release(defaultPrinter) };

        listeners.Append(this);
    }

    OnTestPartResultEventListener::~OnTestPartResultEventListener()
    {
        listeners.Release(this);
    }
}
