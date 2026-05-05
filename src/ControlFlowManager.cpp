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
		auto This = this->shared_from_this();
		timerService->setTimer(1000, [This, taskId]() {
			std::lock_guard<std::mutex> guard(This->lock);
			if (This->currTaskId == taskId) {
				This->blockingTask->terminate();
			}
				
		});
	}

}
std::atomic<int> SpellChessLib::ControlFlowManager::taskIdSeq = 1;