#ifndef SUPPORT_TYPES_HPP
#define SUPPORT_TYPES_HPP

#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include <cstddef>
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
        enum class Ordering
        {
            defined,
            reverse,
        };

        struct Sources
        {
            std::set<std::filesystem::path, std::less<>> paths{};
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
            std::unique_ptr<tag_expression::Expression> retryTagExpression{};
            bool featureHooks{ false };
        } runtime;

        struct RunEnvironment
        {
            std::optional<std::filesystem::path> cwd;
            std::optional<std::span<const char*>> env;
        } runEnvironment;
    };

    struct PickleSource
    {
        std::shared_ptr<const cucumber::messages::Pickle> pickle;
        std::shared_ptr<const cucumber::messages::GherkinDocument> gherkinDocument;
        std::shared_ptr<const cucumber::messages::Location> location;
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
