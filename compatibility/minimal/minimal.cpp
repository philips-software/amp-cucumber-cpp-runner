#include "cucumber_cpp/Steps.hpp"
#include <cstdint>

STEP(R"(I have {int} cukes in my belly)", (std::int32_t number))
{
    // no-op
}
