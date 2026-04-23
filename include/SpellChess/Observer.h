#pragma once

#include <vector>
#include <list>
#include <memory>
#include <mutex>

namespace SpellChessLib {

	template <class State>
	class Observer {
	public:
		virtual void onObservedStateUpdate(const State & state) = 0;
	};

	template <class State>
	class Broadcaster {
		std::list<std::weak_ptr<Observer<State>>> observers;

	protected:
		mutable std::mutex lock;
		void broadcast(const State & value);
	public:
		virtual void addObserver(std::shared_ptr<Observer<State>> observer);
		virtual void removeObserver(std::shared_ptr<Observer<State>> observer);

	};

	template <class State>
	class ObservableState : public Broadcaster<State> {
	protected:
		State observedVariable;
		mutable std::mutex valueLock;
	public:
		ObservableState(const State& s) : observedVariable(s) {};
		void addObserver(std::shared_ptr<Observer<State>> observer) override;
		void set(const State& s);
	};


	template <class State>
	class DefaultNullableObservable : public Broadcaster<State*> {
	protected:
		std::unique_ptr<State> observedVariable;
		mutable std::mutex valueLock;
	public:
		DefaultNullableObservable() : observedVariable(nullptr) {};
		DefaultNullableObservable(const State& s) : observedVariable(new State(s)) {};
		void addObserver(std::shared_ptr<Observer<State*>> observer) override;
		void set(const State& s);
		void set(std::nullptr_t t);
		//void set(const State* s) override;
	};

	template<class State>
	inline void Broadcaster<State>::broadcast(const State & s){
		std::lock_guard<std::mutex> guard(lock);
		for (std::weak_ptr<Observer<State>> weakPtr : observers)
			if (std::shared_ptr<Observer<State>> ptr = weakPtr.lock())
				ptr->onObservedStateUpdate(s);
	}

	template<class State>
	void Broadcaster<State>::addObserver(std::shared_ptr<Observer<State>> observer){
		std::lock_guard<std::mutex> guard(lock);
		observers.push_back(observer);
	}

	template<class State>
	void Broadcaster<State>::removeObserver(std::shared_ptr<Observer<State>> observer){
		std::lock_guard<std::mutex> guard(lock);
		std::vector<typename std::list<std::weak_ptr<Observer<State>>>::iterator> indexesToRemove;
		auto it = observers.begin();
		std::weak_ptr<Observer<State>> currentPtr;
		std::shared_ptr<Observer<State>> lockedPtr;
		while (it != observers.end()) {
			currentPtr = *it;
			if ((lockedPtr = currentPtr.lock()) && lockedPtr.get() == observer.get()) {
				indexesToRemove.push_back(it);
			}
			++it;
		};

		for (auto i : indexesToRemove)
			observers.erase(i);
	}

	template<class State>
	void ObservableState<State>::addObserver(std::shared_ptr<Observer<State>> observer){
		observer->onObservedStateUpdate(observedVariable);
		Broadcaster<State>::addObserver(observer);
	}

	template<class State>
	void ObservableState<State>::set(const State& s){
		std::lock_guard<std::mutex> guard(valueLock);
		observedVariable = s;
		this->broadcast(s);
	}

	template<class State>
	inline void DefaultNullableObservable<State>::addObserver(std::shared_ptr<Observer<State*>> observer){
		observer->onObservedStateUpdate(observedVariable.get());
		Broadcaster<State*>::addObserver(observer);
	}

	template<class State>
	inline void DefaultNullableObservable<State>::set(const State& s){
		std::lock_guard<std::mutex> guard(valueLock);
		observedVariable.reset(new State(s));
		this->broadcast(observedVariable.get());
	}

	template<class State>
	inline void DefaultNullableObservable<State>::set(std::nullptr_t t){
		std::lock_guard<std::mutex> guard(valueLock);
		observedVariable.reset();
		this->broadcast(observedVariable.get());
	}

}