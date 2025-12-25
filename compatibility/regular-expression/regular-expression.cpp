#include "cucumber_cpp/CucumberCpp.hpp"
#include <string>

STEP(R"(^a (.*?)(?: and a (.*?))?(?: and a (.*?))?$)", (const std::string& vegtable1, const std::string& vegtable2, const std::string& vegtable3))
{
    // no-op
}
