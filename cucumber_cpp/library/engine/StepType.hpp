#ifndef ENGINE_STEPTYPE_HPP
#define ENGINE_STEPTYPE_HPP

namespace cucumber_cpp::library::engine
{
    enum struct StepType
    {
        given,
        when,
        then,
        any
    };
}
#endif
