#ifndef CUCUMBER_CPP_HOOKREGISTRY_HPP
#define CUCUMBER_CPP_HOOKREGISTRY_HPP

#include "cucumber-cpp/Body.hpp"
#include "cucumber-cpp/Context.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp
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

    struct TagExpressionMatch
    {
        TagExpressionMatch(const std::string& tagExpression, const std::set<std::string, std::less<>>& tags);

        bool Matched() const;

    private:
        bool matched;
    };

    struct HookTagExpression
    {
        explicit HookTagExpression(std::string tagExpression);

        std::unique_ptr<TagExpressionMatch> Match(const std::set<std::string, std::less<>>& tags) const;
        std::string TagExpression() const;

    private:
        std::string tagExpression;
    };

    struct HookMatch
    {
        std::unique_ptr<TagExpressionMatch> tagExpressionMatch;
        std::unique_ptr<Body> (&factory)(Context& context);
        const HookTagExpression& hookRegex;
    };

    struct HookRegistryBase
    {
        struct Entry
        {
            HookType type;
            HookTagExpression hookTagExpression;
            std::unique_ptr<Body> (&factory)(Context& context);
        };

        std::vector<HookMatch> Query(HookType hookType, const std::set<std::string, std::less<>>& tags) const;

        std::size_t Size() const;
        std::size_t Size(HookType hookType) const;

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
    std::size_t HookRegistryBase::Register(const std::string& matcher, HookType hookType)
    {
        registry.emplace_back(hookType, HookTagExpression{ matcher }, Construct<T>);
        return registry.size();
    }

    template<class T>
    std::unique_ptr<Body> HookRegistryBase::Construct(Context& context)
    {
        return std::make_unique<T>(context);
    }

    template<class T>
    std::size_t HookRegistry::Register(const std::string& matcher, HookType hookType)
    {
        return Instance().HookRegistryBase::Register<T>(matcher, hookType);
    }
}

#endif
