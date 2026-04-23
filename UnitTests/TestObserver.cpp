#include "pch.h"
#include <SpellChess/Observer.h>

using namespace SpellChessLib;


template <class State>
class ExampleObserver : public Observer<State> {
	State knownValue = 0;
	int updateCount = 0;
public:
	State getKnownValue() { return knownValue; }
	int getUpdateCount() { return updateCount; }

	// Odziedziczono za poœrednictwem elementu Observer
	void onObservedStateUpdate(const State& state) override{
		knownValue = state;
		++updateCount;
	}
};


TEST(DefaultObservable, addingObserver) {
	ObservableState<int> integer(2);

	auto observer = std::make_shared<ExampleObserver<int>>();
	integer.addObserver(observer);

	ASSERT_EQ(observer->getKnownValue(), 2);
}

TEST(DefaultObservable, valueChange) {
	ObservableState<int> integer(2);
	auto observer = std::make_shared<ExampleObserver<int>>();
	integer.addObserver(observer);
	integer.set(3);
	ASSERT_EQ(observer->getKnownValue(), 3);
}

TEST(Observable, removeObserver) {
	ObservableState<int> integer(2);
	auto observer = std::make_shared<ExampleObserver<int>>();
	integer.addObserver(observer);
	integer.removeObserver(observer);
	integer.set(4);
	ASSERT_NE(observer->getKnownValue(), 4);
}

TEST(Observable, observerGoesOutOfScope) {
	ObservableState<int> integer(2);
	auto observer = std::make_shared<ExampleObserver<int>>();
	integer.addObserver(observer);
	observer.reset();
	ASSERT_NO_FATAL_FAILURE(integer.set(3));
}

TEST(NullableObservable, defaultValue) {
	DefaultNullableObservable<int> integer;
	auto observer = std::make_shared<ExampleObserver<int*>>();
	integer.addObserver(observer);
	ASSERT_EQ(observer->getKnownValue(), nullptr);
}

TEST(NullableObservable, initialisedWithConcreteValue) {
	DefaultNullableObservable<int> integer(2);
	auto observer = std::make_shared<ExampleObserver<int*>>();
	integer.addObserver(observer);
	ASSERT_NE(observer->getKnownValue(), nullptr);
	ASSERT_EQ(*observer->getKnownValue(), 2);
}

TEST(NullableObservable, set) {
	DefaultNullableObservable<int> integer;
	auto observer = std::make_shared<ExampleObserver<int*>>();
	integer.addObserver(observer);
	integer.set(2);

	ASSERT_NE(observer->getKnownValue(), nullptr);
	ASSERT_EQ(*observer->getKnownValue(), 2);
}

TEST(NullableObservable, setToNull) {
	DefaultNullableObservable<int> integer(1);
	auto observer = std::make_shared<ExampleObserver<int*>>();
	integer.addObserver(observer);
	integer.set(nullptr);
	ASSERT_EQ(observer->getKnownValue(), nullptr);
}