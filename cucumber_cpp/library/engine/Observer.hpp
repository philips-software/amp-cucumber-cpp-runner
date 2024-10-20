#ifndef ENGINE_EVENTSYSTEM_HPP
#define ENGINE_EVENTSYSTEM_HPP

#include <cassert>
#include <list>
#include <stdexcept>

namespace cucumber_cpp::library::engine
{
    template<class TObserverType>
    struct Subject;

    template<class TObserver, class TSubjectType>
    struct Observer
    {
        using SubjectType = TSubjectType;

        Observer() = default;
        Observer(SubjectType& subject);

    protected:
        ~Observer();

    public:
        SubjectType& Subject() const;
        bool Attached() const;

        void Attach(SubjectType& subject);
        void Detach();

    private:
        SubjectType* eventSubject{ nullptr };
    };

    template<class TObserverType>
    struct Subject
    {
    protected:
        Subject() = default;
        ~Subject();

    public:
        using ObserverType = TObserverType;
        using SubjectType = ObserverType::SubjectType;

        void RegisterObserver(Observer<TObserverType, SubjectType>* observer);
        void UnregisterObserver(Observer<TObserverType, SubjectType>* observer);

        template<class Fun>
        void NotifyObservers(const Fun& fun);

        template<class Fun>
        void NotifyObservers(const Fun& fun) const;

        template<class... FunMemArgs>
        void NotifyObservers(void (TObserverType::*funMem)(FunMemArgs&&...), FunMemArgs&&... funMemArgs);

        template<class... FunMemArgs>
        void NotifyObservers(void (TObserverType::*funMem)(FunMemArgs&&...), FunMemArgs&&... funMemArgs) const;

    private:
        std::list<Observer<ObserverType, SubjectType>*> observers;
    };

    //////////////////////////////////////////////////////////////////////////

    template<class TObserver, class TSubjectType>
    Observer<TObserver, TSubjectType>::Observer(TSubjectType& subject)
    {
        Attach(subject);
    }

    template<class TObserver, class TSubjectType>
    Observer<TObserver, TSubjectType>::~Observer()
    {
        Detach();
    }

    template<class TObserver, class TSubjectType>
    TSubjectType& Observer<TObserver, TSubjectType>::Subject() const
    {
        return *eventSubject;
    }

    template<class TObserver, class TSubjectType>
    bool Observer<TObserver, TSubjectType>::Attached() const
    {
        return eventSubject != nullptr;
    }

    template<class TObserver, class TSubjectType>
    void Observer<TObserver, TSubjectType>::Attach(TSubjectType& subject)
    {
        Detach();
        eventSubject = &subject;
        static_cast<cucumber_cpp::library::engine::Subject<TObserver>&>(*eventSubject).RegisterObserver(this);
    }

    template<class TObserver, class TSubjectType>
    void Observer<TObserver, TSubjectType>::Detach()
    {
        if (Attached())
            static_cast<cucumber_cpp::library::engine::Subject<TObserver>&>(*eventSubject).UnregisterObserver(this);
        eventSubject = nullptr;
    }

    template<class TObserverType>
    Subject<TObserverType>::~Subject()
    {
        assert(observers.empty());
    }

    template<class TObserverType>
    void Subject<TObserverType>::RegisterObserver(Observer<TObserverType, SubjectType>* observer)
    {
        observers.push_back(observer);
    }

    template<class TObserverType>
    void Subject<TObserverType>::UnregisterObserver(Observer<TObserverType, SubjectType>* observer)
    {
        observers.remove(observer);
    }

    template<class TObserverType>
    template<class Fun>
    void Subject<TObserverType>::NotifyObservers(const Fun& fun)
    {
        for (auto iter = observers.begin(); iter != observers.end();)
        {
            auto* curr = *iter;
            ++iter;
            fun(static_cast<TObserverType&>(*curr));
        }
    }

    template<class TObserverType>
    template<class Fun>
    void Subject<TObserverType>::NotifyObservers(const Fun& fun) const
    {
        for (auto iter = observers.begin(); iter != observers.end();)
        {
            auto* curr = *iter;
            ++iter;
            fun(static_cast<const TObserverType&>(*curr));
        }
    }

    template<class TObserverType>
    template<class... FunMemArgs>
    void Subject<TObserverType>::NotifyObservers(void (TObserverType::*funMem)(FunMemArgs&&... funMemArgs), FunMemArgs&&... funMemArgs)
    {
        for (auto iter = observers.begin(); iter != observers.end();)
        {
            auto* curr = *iter;
            ++iter;
            (static_cast<TObserverType&>(*curr).*funMem)(std::forward<FunMemArgs>(funMemArgs)...);
        }
    }

    template<class TObserverType>
    template<class... FunMemArgs>
    void Subject<TObserverType>::NotifyObservers(void (TObserverType::*funMem)(FunMemArgs&&... funMemArgs), FunMemArgs&&... funMemArgs) const
    {
        for (auto iter = observers.begin(); iter != observers.end();)
        {
            auto* curr = *iter;
            ++iter;
            (static_cast<const TObserverType&>(*curr).*funMem)(std::forward<FunMemArgs>(funMemArgs)...);
        }
    }
}

#endif
