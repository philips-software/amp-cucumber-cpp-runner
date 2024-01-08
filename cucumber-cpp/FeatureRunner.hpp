#ifndef CUCUMBER_CPP_FEATURERUNNER_HPP
#define CUCUMBER_CPP_FEATURERUNNER_HPP

#include "cucumber-cpp/Hooks.hpp"
#include <string>

namespace cucumber_cpp
{
    struct FeatureRunner
    {
        FeatureRunner(Context& programContext, const std::string& tagExpr);

        void Run(nlohmann::json& json);

    private:
        std::string tagExpr;
        Context featureContext;
    };
}

#endif
