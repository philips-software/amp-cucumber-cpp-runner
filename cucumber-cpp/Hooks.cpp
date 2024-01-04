#include "cucumber-cpp/Hooks.hpp"
#include <utility>

namespace cucumber_cpp
{
    Hooks::Hooks()
    {
        HookRegistration<HookBeforeAll>::RegisterAll(*this);
        HookRegistration<HookAfterAll>::RegisterAll(*this);
        HookRegistration<HookBefore>::RegisterAll(*this);
        HookRegistration<HookAfter>::RegisterAll(*this);
        HookRegistration<HookBeforeStep>::RegisterAll(*this);
        HookRegistration<HookAfterStep>::RegisterAll(*this);
    }

    void Hooks::BeforeAll(Context& context)
    {
        for (const auto& hook : hooksBeforeAll)
        {
            hook->Run(context);
        }
    }

    void Hooks::AfterAll(Context& context)
    {
        for (const auto& hook : hooksAfterAll)
        {
            hook->Run(context);
        }
    }

    void Hooks::BeforeFeature(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksBeforeFeature)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::AfterFeature(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksAfterFeature)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::Before(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksBefore)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::After(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksAfter)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::BeforeStep(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksBeforeStep)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::AfterStep(Context& context, const nlohmann::json& json)
    {
        for (const auto& hook : hooksAfterStep)
        {
            hook->Run(context, json);
        }
    }

    void Hooks::Register(std::unique_ptr<HookBeforeAll> hook)
    {
        hooksBeforeAll.push_back(std::move(hook));
    }

    void Hooks::Register(std::unique_ptr<HookAfterAll> hook)
    {
        hooksAfterAll.push_back(std::move(hook));
    }

    void Hooks::Register(std::unique_ptr<HookBefore> hook)
    {
        hooksBefore.push_back(std::move(hook));
    }

    void Hooks::Register(std::unique_ptr<HookAfter> hook)
    {
        hooksAfter.push_back(std::move(hook));
    }

    void Hooks::Register(std::unique_ptr<HookBeforeStep> hook)
    {
        hooksBeforeStep.push_back(std::move(hook));
    }

    void Hooks::Register(std::unique_ptr<HookAfterStep> hook)
    {
        hooksAfterStep.push_back(std::move(hook));
    }
}
