#include <SpellChess/TimerService.h>
#include <vector>

void SpellChessLib::TimerService::destroyTimer(PlayerTimer_data* timer)
{
	auto it = timers.begin();
	while (it != timers.end())
		if (it->id == timer->id) {
			timers.erase(it);
			break;
		}
		else
			++it;
}

void SpellChessLib::TimerService::refreshTimers(time_t delta_t)
{
	std::lock_guard<std::recursive_mutex> guard(lock);
	std::vector<PlayerTimer_data *> garbage;

	for (auto& timer : timers) {
		std::lock_guard<std::mutex> guard(timer.lock);
		if (!timer.stopped && timer.value > 0) {
			timer.value -= delta_t;
			if (timer.value <= 0) {
				timer.expirationCallback();
				timer.value = 0;
			}
		}

		if (timer.deleteLater)
			garbage.push_back(&timer);
	}

	for (auto t : garbage)
		destroyTimer(t);
}

void SpellChessLib::TimerService::deleteLater(int id){

	for (auto& t : timers) 
	if (t.id == id)
		t.deleteLater = true;
}

SpellChessLib::TimerService::TimerService() {
	worker = std::make_unique<std::thread>([this]() {
		std::chrono::steady_clock::time_point start, end;
		time_t delta_t = 0;
		while (this->running) {
			start = std::chrono::steady_clock::now();
			/*
				Busy waiting is bad, but we really want 1ms precision
			*/
			while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1)) {}
			this->refreshTimers(1);
		}
		}
	);
}

SpellChessLib::TimerService::~TimerService() {
	this->running = false;
	worker->join();
}

std::unique_ptr<SpellChessLib::PlayerTimer> SpellChessLib::TimerService::createTimer(const time_t& initialValue, const std::function<void()>& expirationCallback) {
	std::lock_guard<std::recursive_mutex> guard(lock);
	unsigned int id = idSeq++;
	timers.emplace_back(id, initialValue, expirationCallback);
	return std::make_unique<SpellChessLib::PlayerTimer>(&timers.back(), shared_from_this());
}

void SpellChessLib::TimerService::setTimer(const time_t& initialValue, const std::function<void()>& expirationCallback){
	std::lock_guard<std::recursive_mutex> guard(lock);
	unsigned int id = idSeq++;
	timers.emplace_back(id, initialValue, [this, expirationCallback, id]() {
		expirationCallback();
		this->deleteLater(id);
	});
	timers.back().stopped = false;
}

void SpellChessLib::PlayerTimer::stop() {
	std::lock_guard<std::mutex> guard(data->lock);
	data->stopped = true;
}

void SpellChessLib::PlayerTimer::resume() {
	std::lock_guard<std::mutex> guard(data->lock);
	data->stopped = false;
}

void SpellChessLib::PlayerTimer::extend(const time_t& inc) {
	std::lock_guard<std::mutex> guard(data->lock);
	if (data->value > 0)
		data->value += inc;
}

void SpellChessLib::PlayerTimer::setCallback(std::function<void()> clb) {
	std::lock_guard<std::mutex> guard(data->lock);
	data->expirationCallback = clb;
}

bool SpellChessLib::PlayerTimer::expired() const {
	std::lock_guard<std::mutex> guard(data->lock);
	return data->value <= 0;
}

bool SpellChessLib::PlayerTimer::isStopped() const {
	std::lock_guard<std::mutex> guard(data->lock);
	return data->stopped;
}

time_t SpellChessLib::PlayerTimer::getValue() const {
	std::lock_guard<std::mutex> guard(data->lock);
	return data->value;
}

void SpellChessLib::ClassicTimeController::onMovePlayed() {
	timer->stop();
	timer->extend(inc);
}

void SpellChessLib::ClassicTimeController::resume() {
	timer->resume();
}

void SpellChessLib::ClassicTimeController::pause(){
	timer->stop();
}

void SpellChessLib::ClassicTimeController::setExpirationClb(std::function<void()> clb) {
	timer->setCallback(clb);
}

void SpellChessLib::ClassicTimeController::exportAsGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& builder, const PlayerColor& perspective) const{
	unsigned int base = this->timer->getValue();
	unsigned int inc = this->inc;
	builder.withInfiniteMode(false);
	if (perspective == White) 
		builder.withWhiteTime(base, inc);
	
	if (perspective == Black)
		builder.withBlackTime(base, inc);
}

std::chrono::milliseconds SpellChessLib::ClassicTimeController::getValue() const {
	return std::chrono::milliseconds(timer->getValue());
}

SpellChessLib::PlayerTimerState SpellChessLib::ClassicTimeController::exportState() const {
	return PlayerTimerState {!timer->isStopped(), true, getValue()};
};

void SpellChessLib::InfiniteTimeTimerController::exportAsGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& builder, const PlayerColor& perspective) const {
	/*
		Despite the name we DO NOT want set "infinity" flag to the query. Doing so would require the GUI to send "stop" command to the engine 
		manually. Because we want to engine to send move whenever it wants, we can not send such parameters. Instead, we will rely on external
		user defined params to provide stopping conditions (like depth, movetime or nodes)
	*/
	builder.withInfiniteMode(false);
}

