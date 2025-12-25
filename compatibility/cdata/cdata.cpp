#include "cucumber_cpp/CucumberCpp.hpp"
#include <cstdint>

GIVEN(R"(I have {int} <![CDATA[cukes]]> in my belly)", ([[maybe_unused]] std::int32_t number))
{}
