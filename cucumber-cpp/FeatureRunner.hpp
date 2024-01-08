#ifndef CUCUMBER_CPP_FEATURERUNNER_HPP
#define CUCUMBER_CPP_FEATURERUNNER_HPP

#include "cucumber-cpp/Hooks.hpp"
#include <string>

namespace cucumber_cpp
{
    struct FeatureRunner
    {
        FeatureRunner(Hooks& hooks, Context& programContext, const std::string& tagExpr);

        void Run(nlohmann::json& json);

    private:
        Hooks& hooks;

        std::string tagExpr;
        Context featureContext;
    };
}

#endif
