#ifndef ENGINE_RESULT_HPP
#define ENGINE_RESULT_HPP

namespace cucumber_cpp::library::engine
{
    enum struct Result
    {
        passed,
        skipped,
        pending,
        undefined,
        ambiguous,
        failed
    };
}

#endif
