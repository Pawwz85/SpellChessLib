#include <Core/Observer.h>
#include <gtest/gtest.h>

#include <string>

class ObservableInt : public Core::Observable<int> {
    public:

    ObservableInt(int value = 0) {
        state = value;
    };

    void set(int v) {
        state = v;
        notifyAll();
    }
};

class ConcreteIntObserver : public Core::Observer<int> {
    public:
    int lastValue;
    
    ConcreteIntObserver() : lastValue(0) {};

    void onStateUpdate(const int & s) {
        lastValue = s;
    }

    bool hasValidObservable() const {
        return subject != nullptr;
    }
};

struct CustomType {
    int first;
    std::string second;
    CustomType(int f, std::string second) : first(f), second(second) {};
};

struct CustomTypeObserver : Core::Observer<CustomType> {
    public:
    CustomType lastValue;
    CustomTypeObserver() : lastValue(-1, "empty") {};
    void onStateUpdate(const CustomType & custom) {
        lastValue = custom;
    }
};

namespace Core {

    TEST(Observer, noop) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer; 
        integer->set(2);
        ASSERT_NE(observer.lastValue, 2);
    }

    TEST(Observer, update) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer; 
        integer->attachObserver(observer);
        integer->set(2);
        ASSERT_EQ(observer.lastValue, 2);
    }

    TEST(Observer, dettach) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer; 
        integer->attachObserver(observer);
        integer->dettachObserver(observer);
        integer->set(2);
        ASSERT_NE(observer.lastValue, 2);
        ASSERT_EQ(integer->countObservers(), 0);
    }


    TEST(Observer, attach_observer) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer; 
        integer->attachObserver(observer);
        ASSERT_TRUE(observer.hasValidObservable());
        ASSERT_EQ(integer->countObservers(), 1);
    }

    TEST(Observer, observable_expired) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer; 
        integer->attachObserver(observer);
        integer.reset();
        ASSERT_TRUE(observer.hasValidObservable()); // observer is excpeted to hold to state through shared pointer
    }

    TEST(Observer, observer_expired) {
        auto integer = std::make_shared<ObservableInt>();
        ConcreteIntObserver * observer =  new ConcreteIntObserver();
        integer->attachObserver(*observer);
        delete observer;
        ASSERT_EQ(integer->countObservers(), 0);
    };

    TEST(Observer, observer_registered_mulitple_times) {
        auto int1 = std::make_shared<ObservableInt>();
        auto int2 = std::make_shared<ObservableInt>();
        ConcreteIntObserver observer;
        int1->attachObserver(observer);
        int2->attachObserver(observer);

        ASSERT_EQ(int1->countObservers(), 0);
        ASSERT_EQ(int2->countObservers(), 1);
    }

    TEST(Observer, observer_is_notified_at_attachment) {
        auto integer = std::make_shared<ObservableInt>(7);
        ConcreteIntObserver observer;
        integer->attachObserver(observer);
        ASSERT_EQ(observer.lastValue, 7);
    }

    TEST(Observer, custom_type) {
        auto someObservable = std::make_shared<Observable<CustomType>>(2, "some");
        CustomTypeObserver observer;
        someObservable->attachObserver(observer);
        ASSERT_EQ(observer.lastValue.first, 2);
        ASSERT_EQ(observer.lastValue.second, "some");
    }
    
}