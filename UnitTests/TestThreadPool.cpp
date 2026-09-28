#include "pch.h"
#include "ThreadPoolUtils.h"

TEST(ThreadPool, Init) {
	SpellChessLib::ThreadPool pool(4);
	ASSERT_EQ(pool.getDesiredSize(), 4);
	ASSERT_EQ(pool.getActualSize(), 4);
}

TEST(ThreadPool, IdlePoolUpSizing) {
	SpellChessLib::ThreadPool pool(4);
	pool.setDesiredSize(6);
	ASSERT_EQ(pool.getDesiredSize(), 6);
	ASSERT_EQ(pool.getActualSize(), 6);
}

TEST(ThreadPool, IdlePoolDownSizing) {
	SpellChessLib::ThreadPool pool(4);
	pool.setDesiredSize(2);
	ASSERT_EQ(pool.getDesiredSize(), 2);
	ASSERT_EQ(pool.getActualSize(), 2);
}

#define TASK(boolean) [&boolean]() {while (!boolean) std::this_thread::sleep_for(std::chrono::milliseconds(5));} 

TEST(ThreadPool, busyPoolDownSizing) {
	SpellChessLib::ThreadPool pool(2);
	volatile bool task1Completed = false, task2Completed = false;
	pool.queueTask(TASK(task1Completed));
	pool.queueTask(TASK(task2Completed));
	pool.setDesiredSize(1);
	ASSERT_EQ(pool.getDesiredSize(), 1);
	ASSERT_EQ(pool.getActualSize(), 2);
	task1Completed = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_EQ(pool.getActualSize(), 1);
	task2Completed = true;
}

TEST(ThreadPool, queueingTask) {
	SpellChessLib::ThreadPool pool(2);
	volatile bool task1Completed = false, task2Completed = false, task3Completed = false;

	pool.queueTask(TASK(task1Completed));
	pool.queueTask(TASK(task2Completed));
	pool.queueTask(TASK(task3Completed));

	ASSERT_EQ(pool.countBusyWorkers(), 2);
	ASSERT_EQ(pool.getQueueLength(), 1);
	task1Completed = task2Completed = task3Completed = true;
}

TEST(ThreadPool, forcingTask) {
	SpellChessLib::ThreadPool pool(2);
	volatile bool task1Completed = false, task2Completed = false, task3Completed = false;

	pool.forceTaskExecution(TASK(task1Completed));
	pool.forceTaskExecution(TASK(task2Completed));
	pool.forceTaskExecution(TASK(task3Completed));

	ASSERT_EQ(pool.countBusyWorkers(), 3);
	ASSERT_EQ(pool.getQueueLength(), 0);
	task1Completed = task2Completed = task3Completed = true;
}

TEST(ThreadPool, canNotCreateEmptyPool) {
	ASSERT_THROW(SpellChessLib::ThreadPool pool(0), SpellChessLib::EmptyThreadPoolException);
}

TEST(ThreadPool, canNotSetThreadPoolSizeToZero) {
	SpellChessLib::ThreadPool pool(1);
	ASSERT_THROW(pool.setDesiredSize(0), SpellChessLib::EmptyThreadPoolException);
}
