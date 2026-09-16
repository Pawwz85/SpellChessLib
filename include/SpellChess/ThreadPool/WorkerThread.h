#pragma once 
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>

namespace SpellChessLib {
	
	template <typename Task>
	class WorkerThread {
	public:
		class _WorkerThreadMaster {
		public:
			virtual ~_WorkerThreadMaster() = default;
			virtual void onWorkerBecomingIdle(WorkerThread<Task>* worker) = 0;
		};

		class WorkerBusyException : public std::exception {
		public:
			const char* what() const noexcept override {
				return "Can not assign more work for busy worker";
			}
		};

		enum WorkerStatus {
			Idle,
			Working
		};

	private:
		mutable std::mutex lock;
		std::condition_variable conditional;
		WorkerStatus status  = Idle;
		std::atomic<bool> running;
		std::unique_ptr<Task> currentTask;
		_WorkerThreadMaster* master;
		std::unique_ptr<std::thread> _thread;

		void _main();

	protected:
		virtual void handleTask(Task& t) = 0;

	public:
		WorkerThread(_WorkerThreadMaster* master) : master(master), currentTask(nullptr), running(true) {
			this->_thread = std::make_unique<std::thread>([this]() {this->_main(); });
		};

		virtual ~WorkerThread() {
			stop();
			this->_thread->join();
		}

		WorkerStatus getStatus() const { 
			std::unique_lock<std::mutex> guard(lock);
			return status;
		};
	
		void stop() {
			running = false;
			conditional.notify_one();
		}

		void setTask(std::unique_ptr<Task> task) {
			std::unique_lock<std::mutex> guard(lock);
			
			if (currentTask != nullptr || status == Working) {
				throw WorkerBusyException();
			}

			status = Working;
			currentTask = std::move(task);
			conditional.notify_one();
		}
	};

	template<typename Task>
	inline void WorkerThread<Task>::_main(){
		std::unique_lock<std::mutex> guard(lock);
		guard.unlock();

		std::unique_ptr<Task> localPointer;
		while (running) {
	
			guard.lock();
			if (currentTask == nullptr) {
				//std::this_thread::sleep_for(std::chrono::milliseconds(50));
				goto wait_for_signal;
			}
			localPointer = std::move(currentTask);
			guard.unlock();
			
			try {
				handleTask(*localPointer);
			}
			catch (...) {}; // catch any exception thrown by task handler, so the thread will be ready to be reused later
		
			guard.lock();
			status = Idle;
			guard.unlock();
			master->onWorkerBecomingIdle(this);
			guard.lock();
		wait_for_signal:
			if(running && status != Working)
				conditional.wait(guard);
			guard.unlock();
		}

	}

}