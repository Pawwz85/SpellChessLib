#pragma once

#include <list>
#include <memory>

namespace Core {

    template <class T>
    class Observable;

    template <class T>
    class Observer {
        friend Observable<T>;
        protected:
        std::shared_ptr<Observable<T>> subject;
        virtual void onStateUpdate(const T &) = 0;
        public:
        
        Observer() : subject(nullptr) {};

        virtual ~Observer() {
            if(subject != nullptr)
                subject->dettachObserver(*this);
        }; 
        void notify();
        void setObservedState(std::shared_ptr<Observable<T>> subject);
    };

    template <class T>
    class Observable : public std::enable_shared_from_this<Observable<T>> {
        std::list<Observer<T> *> observers;
        virtual void dettachAll();
        protected:
        T state;
        void notifyAll() const;
        public:

        template <class ... Types>
        Observable(Types... args): state(args...) {};

        virtual ~Observable() {
            dettachAll();
        };
        const T & getState() const;
        void attachObserver(Observer<T> & o);
        void dettachObserver(Observer<T> & o);
        size_t countObservers() const;
    };

    template <class T>
    void Observer<T>::notify() {
        if(subject)
            onStateUpdate(subject->getState());
    }

    template <class T>
    void Observer<T>::setObservedState(std::shared_ptr<Observable<T>> subject){
        if( this->subject != nullptr)
            this->subject->dettachObserver(*this);
        this->subject = subject;
    }

    template <class T>
    void Observable<T>::notifyAll() const {
        for (auto observerPtr: observers) {
                observerPtr->notify();
        }
    }

    template <class T>
    void Observable<T>::dettachAll() {
        for (auto observerPtr: observers){
                observerPtr->subject = nullptr;
            }
        observers.clear();
    };

    template <class T>
    void Observable<T>::attachObserver(Observer<T> & o){
        observers.push_back(&o);
        o.setObservedState(this->shared_from_this());
        o.notify();
    };

    template <class T>
    void Observable<T>::dettachObserver(Observer<T> & o){
        observers.remove(&o);
        o.subject = nullptr;
    };

    template <class T>
    const T & Observable<T>::getState() const {
        return state;
    }

    template <class T>
    size_t Observable<T>::countObservers() const {
        return observers.size();
    }
}

