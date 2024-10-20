#ifndef ENGINE_EVENTS_HPP
#define ENGINE_EVENTS_HPP

#include "cucumber_cpp/library/engine/Observer.hpp"

namespace cucumber_cpp::library::engine
{
    struct EventSubjects;

    struct HookEvents : Observer<HookEvents, EventSubjects>
    {
        using Observer<HookEvents, EventSubjects>::Observer;

    protected:
        ~HookEvents() = default;

    public:
        virtual void HookStarted() = 0;
        virtual void HookFinished() = 0;

        virtual void HookFailure() = 0;
        virtual void HookError() = 0;
    };

    struct FeatureEvents : Observer<FeatureEvents, EventSubjects>
    {
        using Observer<FeatureEvents, EventSubjects>::Observer;

    protected:
        ~FeatureEvents() = default;

    public:
        virtual void FeatureStarted() = 0;
        virtual void FeatureFinished() = 0;
    };

    struct ScenarioEvents : Observer<ScenarioEvents, EventSubjects>
    {
        using Observer<ScenarioEvents, EventSubjects>::Observer;

    protected:
        ~ScenarioEvents() = default;

    public:
        virtual void ScenarioStarted() = 0;
        virtual void ScenarioFinished() = 0;
    };

    struct StepEvents : Observer<StepEvents, EventSubjects>
    {
        using Observer<StepEvents, EventSubjects>::Observer;

    protected:
        ~StepEvents() = default;

    public:
        virtual void StepStarted() = 0;
        virtual void StepFinished() = 0;

        virtual void StepFailure() = 0;
        virtual void StepError() = 0;
    };

    struct EventSubjects
        : Subject<HookEvents>
        , Subject<FeatureEvents>
        , Subject<ScenarioEvents>
        , Subject<StepEvents>
    {
        void NotifyHookStarted();
        void NotifyHookFinished();

        void NotifyHookFailure();
        void NotifyHookError();

        void NotifyFeatureStarted();
        void NotifyFeatureFinished();

        void NotifyScenarioStarted();
        void NotifyScenarioFinished();

        void NotifyStepStarted();
        void NotifyStepFinished();

        void NotifyStepFailure();
        void NotifyStepError();
    };
}

#endif
