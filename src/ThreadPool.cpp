#include <SpellChess/ThreadPool/ThreadPool.h>
#include <SpellChess/ThreadPool/WorkerThread.h>
#include <utility>

using namespace SpellChessLib;

void SpellChessLib::ThreadPool::shrinkPool()
{
	WorkerThread<Task>* task;

	while (desiredSize < workers.size()) {
		task = getAnyIdleWorker();
		if (task != nullptr)
			removeWorker(task);
		else
			break;
	}
}

void ThreadPool::removeWorker(WorkerThread<Task>* worker) {
	worker->stop();
	workers.remove(worker);

	auto other_worker = getAnyIdleWorker();
	auto deletion_task = std::make_unique<std::function<void()>>([worker]() {delete worker; });

	if (other_worker == nullptr)
		taskQueue.emplace(std::move(deletion_task));
	else
		other_worker->setTask(std::move(deletion_task));
}

void ThreadPool::onWorkerBecomingIdle(WorkerThread<Task>* worker)
{
	std::lock_guard<std::mutex> guard(lock);

	if (worker->getStatus() == WorkerThread<Task>::WorkerStatus::Working)
		return;

	if (desiredSize < workers.size()) {
		removeWorker(worker);
		return;
	} 

	if (!taskQueue.empty()) {
		worker->setTask(std::move(taskQueue.front()));
		taskQueue.pop();
	}
	
}

size_t ThreadPool::countBusyWorkers() const {
	std::lock_guard<std::mutex> guard(lock);
	size_t result = 0;

	for (const WorkerThread<Task>* worker : workers)
		if (worker->getStatus() == WorkerThread<Task>::Working)
			++result;

	return result;
}

void ThreadPool::queueTask(const Task & task) {
	std::lock_guard<std::mutex> guard(lock);
	WorkerThread<Task>* idleWorker = getAnyIdleWorker();
	if (idleWorker) {
		idleWorker->setTask(std::make_unique<Task>(task));
	}
	else {
		taskQueue.emplace(std::make_unique<Task>(task));
	}
}

void ThreadPool::forceTaskExecution(const Task & task) {
	std::lock_guard<std::mutex> guard(lock);
	WorkerThread<Task>* idleWorker = getAnyIdleWorker();
	if (idleWorker == nullptr) {
		idleWorker = workerAllocator();
		workers.push_back(idleWorker);
	}

	idleWorker->setTask(std::make_unique<Task>(task));
}

void ThreadPool::setDesiredSize(const size_t& desiredSize) {
	
	if (desiredSize == 0)
		throw EmptyThreadPoolException();
	
	std::lock_guard<std::mutex> guard(lock);
	this->desiredSize = desiredSize;
	if (desiredSize > workers.size())
		expandPool();
	if (desiredSize < workers.size())
		shrinkPool();

}

void SpellChessLib::ThreadPool::assertDesiredSizeIsNotZero() const{
	if (desiredSize == 0)
		throw EmptyThreadPoolException();
}