#include "pch.h"
#include <SpellChess/SpellChessGame.h>
#
using namespace SpellChessLib;


class TimerServiceWithDebugging : public TimerService {
public:
	const PlayerTimer_data* getTimerDataById(unsigned int id) const {
		for (const auto& t : timers)
			if (t.id == id)
				return &t;
		return nullptr;
	};

	PlayerTimer_data* getTimerDataById(unsigned int id) {
		for (auto& t : timers)
			if (t.id == id)
				return &t;
		return nullptr;
	};


	size_t countActiveTimers() const {
		return timers.size();
	}
};

TEST(timer_service, expiration_callback_working) {
	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	bool clb_called = false;
	auto timer = timerService->createTimer(1000, [&clb_called]() {clb_called = true; });
	timer->resume();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	ASSERT_FALSE(clb_called);
	std::this_thread::sleep_for(std::chrono::milliseconds(505));
	ASSERT_TRUE(clb_called);
	ASSERT_TRUE(timer->expired());
}

TEST(timer_service, stop_resume_commands) {
	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	auto timer = timerService->createTimer(500, [](){});
	timer->resume();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	timer->stop();
	auto value = timer->getValue();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ(value, timer->getValue());
	timer->resume();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	ASSERT_NE(value, timer->getValue());
}	

TEST(timer_service, RAII) {
	std::shared_ptr<TimerServiceWithDebugging> timerService = std::make_shared<TimerServiceWithDebugging>();
	bool clb_called = false;
	auto timer = timerService->createTimer(200, [&clb_called]() {clb_called = true; });
	ASSERT_EQ(timerService->countActiveTimers(), 1);
	timer->resume();
	timer.reset();

	std::this_thread::sleep_for(std::chrono::milliseconds(210));
	ASSERT_FALSE(clb_called);
	ASSERT_EQ(timerService->countActiveTimers(), 0);
}

TEST(timer_service, set_timer) {
	std::shared_ptr<TimerServiceWithDebugging> timerService = std::make_shared<TimerServiceWithDebugging>();
	bool clb_called = false;
	timerService->setTimer(200, [&clb_called]() {clb_called = true; });
	ASSERT_EQ(timerService->countActiveTimers(), 1);

	std::this_thread::sleep_for(std::chrono::milliseconds(210));
	ASSERT_TRUE(clb_called);
	ASSERT_EQ(timerService->countActiveTimers(), 0);
}

TEST(timer_factory, infinite) {
	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);
	auto infiniteTimer = factory.makeInfiniteTimerController();
	
	ASSERT_TRUE(true); // place holder for assertions for not implemented behaviour
}

TEST(timer_factory, classic) {
	std::shared_ptr<TimerServiceWithDebugging> timerService = std::make_shared<TimerServiceWithDebugging>();
	TimerControllerFactory factory(timerService);

	auto classicTimerController = factory.makeClassicTimerController(1000, 100);
	auto exposed_timer_data = timerService->getTimerDataById(0);
	ASSERT_NE(exposed_timer_data, nullptr);

	exposed_timer_data->lock.lock();
		ASSERT_EQ(exposed_timer_data->value, 1000);
	exposed_timer_data->lock.unlock();

	classicTimerController->resume();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	exposed_timer_data->lock.lock();
		exposed_timer_data->stopped = true;
		time_t value = exposed_timer_data->value;
	exposed_timer_data->lock.unlock();
	
	classicTimerController->onMovePlayed();

	exposed_timer_data->lock.lock();
	ASSERT_TRUE(exposed_timer_data->stopped);
	ASSERT_EQ(exposed_timer_data->value, value + 100);
	exposed_timer_data->lock.unlock();

	classicTimerController.reset();
}

TEST(ClassicTimeController, exportingToParams) {
	std::shared_ptr<TimerServiceWithDebugging> timerService = std::make_shared<TimerServiceWithDebugging>();
	TimerControllerFactory factory(timerService);
	auto classicTimerController = factory.makeClassicTimerController(1000, 100);
	
}