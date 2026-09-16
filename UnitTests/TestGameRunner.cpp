#include "pch.h"
#include "utils.h"

#include <SpellChess/GameRunner.h>
#include <SpellChess/SpellChessGame.h>
#include <SpellChess/Scheduler/ObjectAllocator.h>
#include <condition_variable>
using namespace SpellChessLib;

TEST(GameRunner, GameIsStillOutgoing) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeInfiniteTimerController()), std::move(factory.makeInfiniteTimerController()));
	
	ASSERT_EQ(game->getWinner(), Nobody);
	ASSERT_EQ(game->getStatusCode(), StillOngoing);
}

class UnRespondingPlayer : public Player {
	bool respectInterrupts = true;
	
	bool interrupted_flag = false;
	bool killed_flag = false;
	bool uciNewGame_flag = false;
	
	std::mutex mut;
	std::condition_variable var;
	void hangForever() {
		std::unique_lock<std::mutex> lock(mut);
		var.wait(lock);
	}

public:

	UnRespondingPlayer(bool allowInterrupts) : respectInterrupts(allowInterrupts) {};

	// Odziedziczono za po�rednictwem elementu Player
	Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override {
		hangForever();
		return { {} };
	}
	Result handlePonderHit() override{
		hangForever();
		return { {} };
	}
	void startPondering(const Game& game, const SpellChessMove& ponderMove) override{
		hangForever();
	}
	void cancelPondering() override{
	}
	bool supportsPondering() const override{
		return true;
	}
	void interrupt() override{
		std::unique_lock<std::mutex> lock(mut);
		
		if (respectInterrupts)
			var.notify_all();
		interrupted_flag = true;
	}

	void kill() override{
		std::unique_lock<std::mutex> lock(mut);
		var.notify_all();
		killed_flag = true;
	}

	bool interrupted() {
		return interrupted_flag;
	}

	bool killed() {
		return killed_flag;
	}

	void onNewGame() override{
		uciNewGame_flag = true;
	}

	bool wasNewGameCalled() {
		return uciNewGame_flag;
	}
};



class SimpleAllocator : public Scheduler::ObjectAllocator<Player>, public std::enable_shared_from_this<SimpleAllocator> {
	std::list<Player*> queue;

	// Odziedziczono za po�rednictwem elementu ObjectAllocator
	void release(Player* p) override
	{
		delete p;
	}

	void destroy(Player* p) override {
		delete p;
	}

public:

	~SimpleAllocator() {
		for (auto p : queue)
			delete p;
	}

	Scheduler::ResourcePointer<Player> retrieve() override{
		Player* p = queue.front();
		queue.pop_front();
		return Scheduler::ResourcePointer<Player>(p, shared_from_this());
	}


	void addPointer(Player* p) {
		queue.push_back(p);
	}
};

TEST(GameRunner, TimedOut) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);
	
	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new UnRespondingPlayer(true));
	simpleAllocator->addPointer(new UnRespondingPlayer(true));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));

	auto whPlayerRaw = (UnRespondingPlayer*)whPlayer.get();
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer));
	runner.run();
	
	ASSERT_EQ(runner.getGame().getWinner(), Black);
	ASSERT_EQ(runner.getGame().getStatusCode(), OpponentTimedOut);
	ASSERT_TRUE(whPlayerRaw->interrupted());
	ASSERT_FALSE(whPlayerRaw->killed());
}

TEST(GameRunner, Kill) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();
	auto whPlayerRaw = (UnRespondingPlayer*)whPlayer.get();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer));
	runner.run();

	ASSERT_EQ(runner.getGame().getWinner(), Black);
	ASSERT_EQ(runner.getGame().getStatusCode(), OpponentTimedOut);
	ASSERT_TRUE(whPlayerRaw->interrupted());
	ASSERT_TRUE(whPlayerRaw->killed());
}

TEST(GameRunner, ucinewgame) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new UnRespondingPlayer(true));
	simpleAllocator->addPointer(new UnRespondingPlayer(true));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();
	auto whPlayerRaw = (UnRespondingPlayer*)whPlayer.get();
	auto blPlayerRaw = (UnRespondingPlayer*)blPlayer.get();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer));
	runner.run();

	ASSERT_TRUE(whPlayerRaw->wasNewGameCalled());
	ASSERT_TRUE(blPlayerRaw->wasNewGameCalled());
}

class RecordedPlayer : public Player {
	std::vector<SpellChessMove> moves;
	size_t index = 0;
	
protected:
	SpellChessMove getNextMove() {
		if (hasNext())
			return moves[index++];
		else return {};
	}
	SpellChessMove peekMove() {
		return moves[index];
	}
	bool hasNext() {
		return index < moves.size();
	}
	size_t currentMoveIndex() const {
		return index;
	}
public:
	RecordedPlayer(const std::vector<SpellChessMove>& moves) : moves(moves) {};
	RecordedPlayer(const std::initializer_list<char*>& moves) {
		for (auto c : moves)
			this->moves.push_back(c);
	}
	// Odziedziczono za po�rednictwem elementu Player
	Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override{
		return {getNextMove()};
	}
	Result handlePonderHit() override{
		return { getNextMove()};
	}
	void startPondering(const Game& game, const SpellChessMove& ponderMove) override {}
	void cancelPondering() override {}
	bool supportsPondering() const override { return false;}
	void interrupt() override{}
	void kill() override{}

	// Odziedziczono za po�rednictwem elementu Player
	void onNewGame() override{}
};



TEST(GameRunner, ToMate) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayer(parseMoveList({ "f2f3", "g2g4" })));
	simpleAllocator->addPointer(new RecordedPlayer(parseMoveList({ "e7e5", "d8h4" })));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();


	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeInfiniteTimerController()), std::move(factory.makeInfiniteTimerController()));
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer));
	runner.run();

	ASSERT_EQ(runner.getGame().getWinner(), Black);
	ASSERT_EQ(runner.getGame().getStatusCode(), CheckMate);
}

TEST(GameRunner, TurnSwitch) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayer(parseMoveList({ "e2e4" })));
	simpleAllocator->addPointer(new UnRespondingPlayer(true));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer));
	runner.run();

	ASSERT_EQ(runner.getGame().getWinner(), White);
	ASSERT_EQ(runner.getGame().getStatusCode(), OpponentTimedOut);
}

class RecordedPlayerWithPondering : public RecordedPlayer {
	std::vector<SpellChessMove> ponderingMoves;

	Result getRecordedResult() {
		SpellChessMove nextMove, ponderMove;
		if (currentMoveIndex() < ponderingMoves.size()) {
			ponderMove = ponderingMoves[currentMoveIndex()];
			nextMove = getNextMove();
			return Result(nextMove, ponderMove);
		}
		return getNextMove();
	}

public:

	RecordedPlayerWithPondering(const std::vector<SpellChessMove>& bestMoves, const std::vector<SpellChessMove>& ponderingMoves) : RecordedPlayer(bestMoves), ponderingMoves(ponderingMoves)
	{
	};

	bool pondering = false;
	int ponderhits = 0;
	int ponderingStarted = 0;
	int ponderingCancelled = 0;

	Result handlePonderHit() override {
		++ponderhits;
		return getRecordedResult();
	}

	Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override {
		return getRecordedResult();
	}

	void startPondering(const Game& game, const SpellChessMove& ponderMove) override {
		++ponderingStarted;
		pondering = true;
	}
	void cancelPondering() override {
		++ponderingCancelled;
		pondering = false;
	}
	bool supportsPondering() const override { return true; }

};

TEST(GameRunner, PonderingOn) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayerWithPondering(parseMoveList({ "e2e4", "d2d4" }), parseMoveList({ "e7e5" })));
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));

	auto whPlayerRaw = (RecordedPlayerWithPondering*)whPlayer.get();
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer), true);
	runner.run();

	ASSERT_EQ(whPlayerRaw->ponderingStarted, 1);
}

TEST(GameRunner, PonderingOff) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayerWithPondering(parseMoveList({ "e2e4", "d2d4" }), parseMoveList({ "e7e5" })));
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));
	
	auto whPlayerRaw = (RecordedPlayerWithPondering*)whPlayer.get();
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer), false);
	runner.run();

	ASSERT_EQ(whPlayerRaw->ponderingStarted, 0);
}

TEST(GameRunner, Ponderhit) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayerWithPondering(parseMoveList({ "e2e4", "d2d4" }), parseMoveList({ "e7e5" })));
	simpleAllocator->addPointer(new RecordedPlayer(parseMoveList({ "e7e5" })));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));

	auto whPlayerRaw = (RecordedPlayerWithPondering*)whPlayer.get();
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer), true);
	runner.run();

	ASSERT_EQ(whPlayerRaw->ponderhits, 1);
	ASSERT_EQ(whPlayerRaw->ponderingCancelled, 0);
}

TEST(GameRunner, Pondermiss) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen(START_POS);
	auto timerService = std::make_shared<TimerService>();
	TimerControllerFactory factory(timerService);

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new RecordedPlayerWithPondering(parseMoveList({ "e2e4", "d2d4" }), parseMoveList({ "e7e5" })));
	simpleAllocator->addPointer(new RecordedPlayer(parseMoveList({ "d7d5" })));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	std::unique_ptr<Game> game = std::make_unique<Game>(state, std::move(factory.makeClassicTimerController(100, 0, 0)), std::move(factory.makeClassicTimerController(100, 0, 0)));
	auto whPlayerRaw = (RecordedPlayerWithPondering*)whPlayer.get();
	GameRunner runner(timerService, std::move(game), std::move(whPlayer), std::move(blPlayer), true);
	runner.run();

	ASSERT_EQ(whPlayerRaw->ponderhits, 0);
	ASSERT_EQ(whPlayerRaw->ponderingCancelled, 1);
}

TEST(GameRunner, fromConfig) {
	auto timerService = std::make_shared<TimerService>();

	GameRunnerConfigurationBuilder builder;
	auto config = builder.withClassicalTimer(100, 10, 0).build();

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	ASSERT_NO_THROW(GameRunner runner(timerService, config, std::move(whPlayer), std::move(blPlayer)));
}

TEST(GameRunner, invalidFenThrowsException) {
	auto timerService = std::make_shared<TimerService>();

	GameRunnerConfigurationBuilder builder;
	auto config = builder.withClassicalTimer(100, 10, 0).withPosition("test").build();

	std::shared_ptr<SimpleAllocator> simpleAllocator = std::make_shared<SimpleAllocator>();
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	simpleAllocator->addPointer(new UnRespondingPlayer(false));
	Scheduler::ResourcePointer<Player> whPlayer = simpleAllocator->retrieve();
	Scheduler::ResourcePointer<Player> blPlayer = simpleAllocator->retrieve();

	ASSERT_THROW(GameRunner runner(timerService, config, std::move(whPlayer), std::move(blPlayer)), FenValidator::InvalidFenException);
}

