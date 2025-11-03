#ifndef CUCUMBER_CPP_RUNNER_SOURCEINFO_H
#define CUCUMBER_CPP_RUNNER_SOURCEINFO_H

#include "cucumber/messages/source.hpp"

#include <filesystem>

namespace cucumber_cpp::library::engine
{
    struct SourceInfo
    {
        SourceInfo(std::filesystem::path path, cucumber::messages::source  source);

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::string ToJson() const;

    private:
        const cucumber::messages::source source;
        std::filesystem::path path;
    };

}

#endif
