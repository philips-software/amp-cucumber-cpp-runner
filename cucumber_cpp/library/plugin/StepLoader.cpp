#include "cucumber_cpp/library/plugin/StepLoader.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"

namespace cucumber_cpp::library::plugin
{
    void StepLoader::Load(support::StepRegistry& stepRegistry)
    {
        stepRegistry.LoadSteps();
    }
}
