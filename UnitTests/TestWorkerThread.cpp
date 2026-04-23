#include "pch.h"

#include <SpellChess/ThreadPool/WorkerThread.h>
#include "ThreadPoolUtils.h"

using namespace SpellChessLib; 

using Task = bool*;


TEST(WorkerThread, Freshly_Created_Worker_Is_Idle) {
	MockedMaster master;
	ConcreteWorker worker(& master);

	ASSERT_EQ(worker.getStatus(), ConcreteWorker::Idle);
}

TEST(WorkerThread, addedTask) {
	MockedMaster master;
	bool taskCompleted = false;
	ConcreteWorker worker(&master);

	worker.setTask(std::make_unique<bool*>(&taskCompleted));
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	ASSERT_EQ(worker.getStatus(), ConcreteWorker::Working);
	taskCompleted = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	ASSERT_EQ(worker.getStatus(), ConcreteWorker::Idle);
	ASSERT_TRUE(master.callbackWasCalled());
}

TEST(WorkerThread, busyWorkerException) {
	MockedMaster master;
	bool taskCompleted = false;
	ConcreteWorker worker(&master);
	worker.setTask(std::make_unique<bool*>(&taskCompleted));
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	ASSERT_THROW(worker.setTask(std::make_unique<bool*>(&taskCompleted)), ConcreteWorker::WorkerBusyException);
	taskCompleted = true;
}