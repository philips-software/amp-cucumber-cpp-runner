#ifndef ENGINE_MISSINGSTEPINFO_HPP
#define ENGINE_MISSINGSTEPINFO_HPP

#include "cucumber-cpp/engine/StepInfoBase.hpp"

namespace cucumber_cpp::engine
{
    struct MissingStepInfo : StepInfoBase
    {
        using StepInfoBase::StepInfoBase;
    };
}

#endif
