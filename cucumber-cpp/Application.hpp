#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <filesystem>
#include <map>
#include <span>
#include <string_view>
#include <vector>

namespace cucumber_cpp
{
    struct Application
    {
        Application(std::span<const char*> args);

        [[nodiscard]] const std::vector<std::string_view>& GetForwardArgs() const;

        void RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());
        void GenerateReports(const std::map<std::string_view, report::Report&>& additionalReports = {});

        [[nodiscard]] int GetExitCode() const;

    private:
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;

        struct Options
        {
            explicit Options(std::span<const char*> args);

            std::vector<std::string_view> tags{};
            std::vector<std::string_view> features{};
            std::vector<std::string_view> reports{};
            std::vector<std::string_view> forwardArgs{};
        };

        Options options;
        nlohmann::json root;

        cucumber::gherkin::app app;
        cucumber::gherkin::app::callbacks cbs;
    };
}

#endif
