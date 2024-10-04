#ifndef ENGINE_EXECUTIONENGINE_HPP
#define ENGINE_EXECUTIONENGINE_HPP

#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include <utility>
#include <vector>

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
