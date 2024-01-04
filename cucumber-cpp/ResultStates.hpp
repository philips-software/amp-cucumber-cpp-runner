#ifndef CUCUMBER_CPP_RESULTSTATES_HPP
#define CUCUMBER_CPP_RESULTSTATES_HPP

#include <array>

namespace cucumber_cpp
{
    namespace result
    {
        constexpr static auto success = "success";
        constexpr static auto skipped = "skipped";
        constexpr static auto failed = "failed";
        constexpr static auto error = "error";
        constexpr static auto pending = "pending";
        constexpr static auto undefined = "undefined";

        constexpr static std::array failStates{
            failed,
            error,
            pending,
            undefined,
        };
    }
}

#endif
