#ifndef UTIL_TRANSFORM_HOOK_DATA_HPP
#define UTIL_TRANSFORM_HOOK_DATA_HPP

#include "cucumber/messages/hook.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::hook TransformHookData(const HookData& hookData);
}

#endif
