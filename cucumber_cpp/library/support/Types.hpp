#ifndef SUPPORT_TYPES_HPP
#define SUPPORT_TYPES_HPP

#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace cucumber_cpp::library::support
{
    struct RunOptions
    {
        struct Sources
        {
            std::span<const std::filesystem::path> paths;
            std::string_view tagExpression;
        } sources;

        struct Runtime
        {
            bool dryRun;
            bool failFast;
            std::size_t retry;
            bool strict;
        } runtime;

        struct RunEnvironment
        {
            std::optional<std::filesystem::path> cwd;
            std::optional<std::span<const char*>> env;
        } runEnvironment;
    };

    struct PickleSource
    {
        std::shared_ptr<const cucumber::messages::pickle> pickle;
        std::shared_ptr<const cucumber::messages::gherkin_document> gherkinDocument;
        std::shared_ptr<const cucumber::messages::location> location;
    };

    struct Runtime
    {
        virtual ~Runtime() = default;
        virtual bool Run() = 0;
    };

    struct RuntimeAdapter
    {
        virtual ~RuntimeAdapter() = default;
        virtual bool Run(std::span<const assemble::AssembledTestSuite> assembledTestSuites) = 0;
    };

}

#endif
