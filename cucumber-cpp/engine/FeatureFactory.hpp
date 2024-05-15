#ifndef ENGINE_FEATUREFACTORY_HPP
#define ENGINE_FEATUREFACTORY_HPP

#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber/gherkin/app.hpp"

namespace cucumber_cpp::engine
{
    struct FeatureTreeFactory
    {
        std::unique_ptr<FeatureInfo> Create(const std::filesystem::path& path, std::string_view tagExpression);

    private:
        cucumber::gherkin::app gherkin;
    };
}

#endif
