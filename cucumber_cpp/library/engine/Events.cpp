#include "cucumber_cpp/library/engine/Events.hpp"
#include "cucumber_cpp/library/engine/Observer.hpp"

namespace cucumber_cpp::library::engine
{
    void EventSubjects::NotifyFeatureStarted()
    {
        Subject<FeatureEvents>::NotifyObservers(&FeatureEvents::FeatureStarted);
    }

    void EventSubjects::NotifyFeatureFinished()
    {
        Subject<FeatureEvents>::NotifyObservers(&FeatureEvents::FeatureFinished);
    }

    void EventSubjects::NotifyScenarioStarted()
    {
        Subject<ScenarioEvents>::NotifyObservers(&ScenarioEvents::ScenarioStarted);
    }

    void EventSubjects::NotifyScenarioFinished()
    {
        Subject<ScenarioEvents>::NotifyObservers(&ScenarioEvents::ScenarioFinished);
    }

    void EventSubjects::NotifyStepStarted()
    {
        Subject<StepEvents>::NotifyObservers(&StepEvents::StepStarted);
    }

    void EventSubjects::NotifyStepFinished()
    {
        Subject<StepEvents>::NotifyObservers(&StepEvents::StepFinished);
    }
}
