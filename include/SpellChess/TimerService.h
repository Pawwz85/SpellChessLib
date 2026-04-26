#pragma once

#include <mutex>
#include <functional>
#include <list>
#include <UCILoader/UCI.h>
#include "Board.h" // for SpellChessMove 

namespace SpellChessLib {
	struct PlayerTimer_data {
		mutable std::mutex lock;
		unsigned int id;
		time_t value;
		std::function<void()> expirationCallback;
		bool stopped;
		bool deleteLater;

		PlayerTimer_data(unsigned int id, time_t value, std::function<void()> clb) : id(id), value(value), expirationCallback(clb), stopped(true), deleteLater(false) {};
	};

	class PlayerTimer;

	class TimerService : public std::enable_shared_from_this<TimerService> {
		unsigned int idSeq = 0;
		bool running = true;

		void destroyTimer(PlayerTimer_data* timer);
		void refreshTimers(time_t delta_t);
		void deleteLater(int id);

		friend PlayerTimer;
		std::unique_ptr<std::thread> worker;

	protected:

		mutable std::recursive_mutex lock;
		std::list<SpellChessLib::PlayerTimer_data> timers;

	public:
		TimerService();
		~TimerService();
		std::unique_ptr<SpellChessLib::PlayerTimer> createTimer(const time_t& initialValue, const std::function<void()>& expirationCallback);
		void setTimer(const time_t& initialValue, const std::function<void()>& expirationCallback);
	};


	class PlayerTimer {
		PlayerTimer_data* data;
		std::shared_ptr<TimerService> timerService;
	public:
		PlayerTimer(PlayerTimer_data* data, std::shared_ptr<TimerService> timerService) : data(data), timerService(timerService) {};
		PlayerTimer(PlayerTimer&& other) noexcept : data(other.data), timerService(other.timerService) { other.data = nullptr; };
		~PlayerTimer() { if (data) timerService->destroyTimer(data); }
		void stop();
		void resume();
		void extend(const time_t& inc);
		void setCallback(std::function<void()> clb);
		bool expired() const;
		bool isStopped() const;
		time_t getValue() const;
	};

	class TimerController {
	public:
		virtual ~TimerController() {};
		virtual void onMovePlayed() = 0;
		virtual void resume() = 0;
		virtual void pause() = 0;
		virtual void setExpirationClb(std::function<void()> clb) = 0;
		virtual void exportAsGoParams(UCILoader::GoParamsBuilder<SpellChessMove> & builder, const PlayerColor & perspective) const = 0;
		virtual std::chrono::milliseconds getValue() = 0;
	};

	// noop controller for infinite time
	class InfiniteTimeTimerController : public TimerController {
	public:
		void onMovePlayed() override {};
		void resume() override {};
		void pause() override {};
		void setExpirationClb(std::function<void()> clb) override {};
		void exportAsGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& builder, const PlayerColor& perspective) const override;
		std::chrono::milliseconds getValue() override { return std::chrono::milliseconds(0); }

	};

	class ClassicTimeController : public TimerController {
		time_t inc;
		std::unique_ptr<PlayerTimer> timer;
	public:
		ClassicTimeController(std::shared_ptr<TimerService> serv, const time_t& base, const time_t& inc) : inc(inc), timer(serv->createTimer(base, []() {})) {};
		void onMovePlayed() override;
		void resume() override;
		void pause() override;
		void setExpirationClb(std::function<void()> clb) override;
		void exportAsGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& builder, const PlayerColor& perspective) const override;
		std::chrono::milliseconds getValue() override;

	};

	class TimerControllerFactory {
		std::shared_ptr<TimerService> timerService;
	public:
		TimerControllerFactory(std::shared_ptr<TimerService> service) : timerService(service) {};
		std::unique_ptr<TimerController> makeInfiniteTimerController() { return std::make_unique<InfiniteTimeTimerController>(); }
		std::unique_ptr<TimerController> makeClassicTimerController(const time_t& base, const time_t& inc) {
			return std::make_unique<ClassicTimeController>(timerService, base, inc);
		}

	};
}