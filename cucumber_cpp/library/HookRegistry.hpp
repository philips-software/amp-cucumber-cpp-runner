#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/tag.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library
{

    template<class T>
    std::unique_ptr<Body> HookBodyFactory(Context& context)
    {
        return std::make_unique<T>(context);
    }

    enum struct HookType
    {
        beforeAll,
        afterAll,
        beforeFeature,
        afterFeature,
        before,
        after,
        beforeStep,
        afterStep,
    };

    struct HookBase
    {
        explicit HookBase(Context& context);

        virtual ~HookBase() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }

    protected:
        Context& context;
    };

    using HookFactory = std::unique_ptr<Body> (&)(Context& context);

    struct HookMatch
    {
        explicit HookMatch(HookFactory factory)
            : factory(factory)
        {}

        HookFactory factory;
    };

    struct HookRegistry
    {
        struct Definition
        {
            Definition(std::string id, HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation)
                : type{ type }
                , tagExpression{ tag_expression::Parse(expression) }
                , factory{ factory }
                , hook{
                    .id = id,
                    .source_reference = cucumber::messages::source_reference{
                        .uri = sourceLocation.file_name(),
                        .location = cucumber::messages::location{
                            .line = sourceLocation.line(),
                        },
                    },
                    .tag_expression = std::string{ expression },
                }
            {}

            HookType type;
            std::unique_ptr<tag_expression::Expression> tagExpression;
            HookFactory factory;
            cucumber::messages::hook hook;
        };

        explicit HookRegistry();

        std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::pickle_tag> tags = {}) const;
        std::vector<std::string> FindIds(HookType hookType, std::span<const cucumber::messages::tag> tags) const;

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(HookType hookType) const;

        HookFactory GetFactoryById(std::string id) const;
        const Definition& GetDefinitionById(std::string id) const;

    private:
        void Register(HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation);

        std::uint32_t nextId{ 1 };
        std::map<std::string, Definition> registry;
    };

    struct HookRegistration
    {
    private:
        HookRegistration() = default;

    public:
        static inline HookRegistration& Instance()
        {
            static HookRegistration instance;
            return instance;
        }

        struct Entry
        {
            Entry(HookType type, std::string_view expression, HookFactory factory, std::source_location sourceLocation)
                : type(type)
                , expression{ expression }
                , factory(factory)
                , sourceLocation{ sourceLocation }
            {}

            HookType type;
            std::string_view expression;
            HookFactory factory;
            std::source_location sourceLocation;
        };

        template<class T>
        static std::size_t Register(std::string_view tagExpression, HookType hookType, std::source_location sourceLocation = std::source_location::current());

        std::span<Entry> GetEntries();
        [[nodiscard]] std::span<const Entry> GetEntries() const;

    private:
        std::vector<Entry> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t HookRegistration::Register(std::string_view tagExpression, HookType hookType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(hookType, tagExpression, HookBodyFactory<T>, sourceLocation);
        return Instance().registry.size();
    }
}

#endif
