#pragma once

#include <memory>
#include <mutex>
#include <atomic>
#include "TimerService.h"

namespace SpellChessLib {
	
	/*
		This class is responsible for monitoring status of control flow of the managed thread.
	*/
		
	class ControlFlowManager : public std::enable_shared_from_this<ControlFlowManager> {
	public:
		class BlockingTask;
	private:
		mutable std::mutex lock;
		std::shared_ptr<TimerService> timerService;
		BlockingTask * blockingTask = nullptr;

		int currTaskId = -1;
		static std::atomic<int> taskIdSeq;

		std::chrono::milliseconds timeBeforeKilled = std::chrono::milliseconds(1000);
		std::chrono::milliseconds latencyMargin = std::chrono::milliseconds(50);

		int allocTaskId() {
			return taskIdSeq++;
		}

	public:

		ControlFlowManager(std::shared_ptr<TimerService> timerService) : timerService(timerService) {};

		class BlockingTask {
			int taskId;
		public:
			virtual void interrupt() = 0;
			virtual void terminate() = 0;
			void setTaskId(int taskId) { this->taskId = taskId; };
			int getTaskId() const { return taskId; }
		};

		void registerBlockingTask(BlockingTask* task);
		void unregisterBlockingTask();
		void askBlockingTaskToReturnControlToParentThread();
	};

}