#pragma once 
#include <mutex>
#include <memory>
#include <queue>
#include <list>
#include <functional>

#include "WorkerThread.h"

namespace SpellChessLib {

	class EmptyThreadPoolException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Setting size of ThreadPool to 0 is not permitted";
		}
	};


	class ThreadPool : public WorkerThread<std::function<void()>>::_WorkerThreadMaster {

		using Task = std::function<void()>;

		class ConcreteWorker : public WorkerThread<Task> {
			void handleTask(Task& t) override {
				t();
			}
		public:
			ConcreteWorker(WorkerThread::_WorkerThreadMaster* master) : WorkerThread(master) {};
		};

		size_t desiredSize;
		
		std::queue<std::unique_ptr<Task>> taskQueue;
		std::list<WorkerThread<Task>*> workers;

		WorkerThread<Task>* getAnyIdleWorker();
		void expandPool();
		void shrinkPool();
		void removeWorker(WorkerThread<Task>* worker);
		void onWorkerBecomingIdle(WorkerThread<Task>* worker) override;

		void assertDesiredSizeIsNotZero() const;

	protected:
		mutable std::mutex lock;

		WorkerThread<Task>* workerAllocator() {
			return new ConcreteWorker(this);
		}

	public:

		ThreadPool(const size_t& targetSize) : desiredSize(targetSize) {
			assertDesiredSizeIsNotZero();
			expandPool();
		};


		virtual ~ThreadPool() {

			// it is important that all tasks are processed before terminating workers, as ThreadPool itself delegates some of memory cleaning work to its workers
			while (!taskQueue.empty() || countBusyWorkers()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		

			for (WorkerThread<Task>* worker : workers) {
				delete worker;
			}
		}
		const size_t& getDesiredSize() const { return desiredSize; };
		size_t getActualSize() const { return workers.size(); };
		size_t countBusyWorkers() const;
		size_t getQueueLength() const { return taskQueue.size(); };
		void queueTask(const Task & task);
		void forceTaskExecution(const Task & task);
		void setDesiredSize(const size_t& desiredSize);
	};

	inline WorkerThread<std::function<void()>>* ThreadPool::getAnyIdleWorker() {
		for (auto worker : workers)
			if (worker->getStatus() == WorkerThread<Task>::WorkerStatus::Idle)
				return worker;

		return nullptr;
	}


	inline void ThreadPool::expandPool() {
		while (workers.size() < desiredSize)
			workers.push_back(workerAllocator());
	}

}