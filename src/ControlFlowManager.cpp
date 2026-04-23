#include <SpellChess/ControlFlowManager.h>

void SpellChessLib::ControlFlowManager::registerBlockingTask(BlockingTask* task){
	std::lock_guard<std::mutex> guard(lock);
	blockingTask = task;
	currTaskId = allocTaskId();
	blockingTask->setTaskId(currTaskId);
}

void SpellChessLib::ControlFlowManager::unregisterBlockingTask(){
	std::lock_guard<std::mutex> guard(lock);

	currTaskId = -1;
	blockingTask = nullptr;
}

void SpellChessLib::ControlFlowManager::askBlockingTaskToReturnControlToParentThread(){
	std::lock_guard<std::mutex> guard(lock);

	int taskId = currTaskId;
	if (blockingTask != nullptr) {
		blockingTask->interrupt();
		
		timerService->setTimer(1000, [this, taskId]() {
			std::lock_guard<std::mutex> guard(this->lock);
			if (this->currTaskId == taskId) {
				this->blockingTask->terminate();
			}
				
		});
	}

}
std::atomic<int> SpellChessLib::ControlFlowManager::taskIdSeq = 1;