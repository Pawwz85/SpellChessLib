#pragma once

#include <SpellChess/ThreadPool/ThreadPool.h>
#include <SpellChess/ThreadPool/WorkerThread.h>

using Task = bool*;

class MockedMaster : public SpellChessLib::WorkerThread<Task>::_WorkerThreadMaster {
	mutable std::mutex lock;
	bool callbackCalled = false;
public:

	bool callbackWasCalled() const {
		std::lock_guard<std::mutex> g(lock);
		return callbackCalled;
	}
	// Odziedziczono za poœrednictwem elementu _WorkerThreadMaster
	void onWorkerBecomingIdle(SpellChessLib::WorkerThread<Task>* worker) override {
		std::lock_guard<std::mutex> g(lock);
		callbackCalled = true;
	}
};

class ConcreteWorker : public SpellChessLib::WorkerThread<Task> {

	// Odziedziczono za poœrednictwem elementu WorkerThread
	void handleTask(Task& t) override
	{
		while (*t == false) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
public:
	ConcreteWorker(SpellChessLib::WorkerThread<Task>::_WorkerThreadMaster* master) : SpellChessLib:: WorkerThread<Task>(master) {};
};
