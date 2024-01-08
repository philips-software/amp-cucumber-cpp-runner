#ifndef CUCUMBER_CPP_CUCUMBERRUNNER_HPP
#define CUCUMBER_CPP_CUCUMBERRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"
#include "nlohmann/json_fwd.hpp"
#include <memory>
#include <string>

namespace cucumber_cpp
{
    struct CucumberRunner
    {
        CucumberRunner(const std::vector<std::string_view>& args, const std::string& tagExpr, std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        void Run(nlohmann::json& json);

    private:
        std::string tagExpr;
        Context programContext;
    };
}

#endif
