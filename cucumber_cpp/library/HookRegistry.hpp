#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/tag_expression/TagExpressionParser.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library
{
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

    struct HookMatch
    {
        explicit HookMatch(std::unique_ptr<Body> (&factory)(Context& context))
            : factory(factory)
        {}

        std::unique_ptr<Body> (&factory)(Context& context);
    };

    struct HookRegistryBase
    {
        struct Entry
        {
            Entry(HookType type, std::string_view expression, std::unique_ptr<Body> (&factory)(Context& context))
                : type(type)
                , tagExpression{ tag_expression::TagExpressionParser{}.Parse(expression) }
                , factory(factory)
            {}

            HookType type;
            std::unique_ptr<tag_expression::Expression> tagExpression;
            std::unique_ptr<Body> (&factory)(Context& context);
        };

        [[nodiscard]] std::vector<HookMatch> Query(HookType hookType, const std::set<std::string, std::less<>>& tags) const;

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] std::size_t Size(HookType hookType) const;

    protected:
        template<class T>
        std::size_t Register(const std::string& tagExpression, HookType hookType);

    private:
        template<class T>
        static std::unique_ptr<Body> Construct(Context& context);

        std::vector<Entry> registry;
    };

    struct HookRegistry : HookRegistryBase
    {
    private:
        HookRegistry() = default;

    public:
        static HookRegistry& Instance();

        template<class T>
        static std::size_t Register(const std::string& tagExpression, HookType hookType);
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t HookRegistryBase::Register(const std::string& tagExpression, HookType hookType)
    {
        registry.emplace_back(hookType, tagExpression, Construct<T>);
        return registry.size();
    }

    template<class T>
    std::unique_ptr<Body> HookRegistryBase::Construct(Context& context)
    {
        return std::make_unique<T>(context);
    }

    template<class T>
    std::size_t HookRegistry::Register(const std::string& tagExpression, HookType hookType)
    {
        return Instance().HookRegistryBase::Register<T>(tagExpression, hookType);
    }
}

#endif
