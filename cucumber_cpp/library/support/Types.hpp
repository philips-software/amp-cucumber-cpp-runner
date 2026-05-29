#ifndef SUPPORT_TYPES_HPP
#define SUPPORT_TYPES_HPP

#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <span>

namespace cucumber_cpp::library::support
{
    struct RunOptions
    {
        enum class Ordering : std::uint8_t
        {
            defined,
            reverse,
        };

        struct Sources
        {
            std::set<std::filesystem::path, std::less<>> paths;
            std::unique_ptr<tag_expression::Expression> tagExpression;
            Ordering ordering{ Ordering::defined };

        } sources;

        struct Runtime
        {
            bool dryRun{ false };
            bool failFast{ false };
            bool failGlobalHookFast{ false };
            std::size_t retry{ 0 };
            bool strict{ true };
            std::unique_ptr<tag_expression::Expression> retryTagExpression;
            bool featureHooks{ false };
            std::uint32_t parallel{ 0 };
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
        virtual bool Run() = 0;
    };

}

#endif
