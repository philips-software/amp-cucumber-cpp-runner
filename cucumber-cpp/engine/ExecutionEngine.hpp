#ifndef ENGINE_EXECUTIONENGINE_HPP
#define ENGINE_EXECUTIONENGINE_HPP

#include "cucumber-cpp/engine/FeatureInfo.hpp"

namespace cucumber_cpp::engine
{
    struct ExecutionEngine
    {
        void Execute(std::vector<FeatureInfo>&& features)
        {
            this->features = std::move(features);
        }

        std::vector<FeatureInfo> features;
    };
}

#endif
