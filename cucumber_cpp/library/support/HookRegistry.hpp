#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/HookFactory.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::support
{

    struct HookMatch
    {
        explicit HookMatch(util::HookFactory factory)
            : factory(factory)
        {}

        util::HookFactory factory;
    };

    struct HookRegistry
    {
        struct Definition
        {
            Definition(std::string id, util::HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, util::HookFactory factory, std::source_location sourceLocation);

            util::HookData data;

            std::unique_ptr<tag_expression::Expression> tagExpression;
            util::HookFactory factory;
        };

        explicit HookRegistry(cucumber::gherkin::id_generator_ptr idGenerator);

        void LoadHooks();

        [[nodiscard]] std::vector<std::string> FindIds(util::HookType hookType, const std::set<std::string, std::less<>>& tags = {}) const;

        [[nodiscard]] std::vector<util::HookData> HooksByType(util::HookType hookType) const;

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] util::HookFactory GetFactoryById(const std::string& id) const;
        [[nodiscard]] const Definition& GetDefinitionById(const std::string& id) const;

    private:
        void Register(const std::string& id, util::HookType type, std::optional<std::string_view> expression, std::optional<std::string_view> name, util::HookFactory factory, std::source_location sourceLocation);

        cucumber::gherkin::id_generator_ptr idGenerator;
        std::map<std::string, Definition, std::less<>> registry;
    };
}

#endif
