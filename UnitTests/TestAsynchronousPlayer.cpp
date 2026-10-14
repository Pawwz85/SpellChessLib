#include <gtest/gtest.h>
#include <SpellChess/AsynchronousPlayer.h>
#include <SpellChess/TimerService.h>
#include <SpellChess/Player.h>
#include <SpellChess/Board.h>
#include <SpellChess/SpellChessGame.h>
#include <thread>
#include <memory>
#include <chrono>

SpellChessLib::Game
gameFixture() {
    SpellChessLib::FenParser parser;
    SpellChessLib::GameState initialState = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    return SpellChessLib::Game(initialState, std::make_unique<SpellChessLib::InfiniteTimeTimerController>(), std::make_unique<SpellChessLib::InfiniteTimeTimerController>());
}

TEST(AsynchronousPlayer, make_result) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread resolveThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        player.resolveCurrentMovePromise("e2e4");
    });

    auto result = player.makeResult(game, std::chrono::milliseconds(100));
    resolveThread.join();
    ASSERT_EQ(result.getBestMove(), "e2e4");
};

TEST(AsynchronousPlayer, make_result_timeout) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    auto result = player.makeResult(game, std::chrono::milliseconds(100));
    ASSERT_EQ(result.getBestMove().from, SpellChessLib::NoSquare);
};

TEST(AsynchronousPlayer, calling_make_result_twice_throws) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread secondaryThread([&player, &game]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
           ASSERT_ANY_THROW(player.makeResult(game, std::chrono::milliseconds(100)));
    });

    auto result = player.makeResult(game, std::chrono::milliseconds(100));
    secondaryThread.join();
};

TEST(AsynchronousPlayer, resolving_move_promise_without_previous_call_throws) {
    SpellChessLib::AsynchronousPlayer player;
    ASSERT_ANY_THROW(player.resolveCurrentMovePromise("e2e4"));
};

TEST(AsynchronousPlayer, interrupt_sets_exception_on_promise) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread interruptThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        player.interrupt();
    });

    ASSERT_ANY_THROW(player.makeResult(game, std::chrono::milliseconds(200)));
    interruptThread.join();

};

TEST(AsynchronousPlayer, resolve_move_promise_after_interrupt_no_throws) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread interruptThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        player.interrupt();
    });

    std::thread resolvingThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ASSERT_NO_THROW(player.resolveCurrentMovePromise("e2e4"));
    });

    try {
        (void)player.makeResult(game, std::chrono::milliseconds(200));
    } catch(...){};

    interruptThread.join();
    resolvingThread.join();
};

TEST(AsynchronousPlayer, kill_sets_exception_on_promise) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread killThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        player.kill();
    });

    ASSERT_ANY_THROW(player.makeResult(game, std::chrono::milliseconds(200)));
    killThread.join();
};

TEST(AsynchronousPlayer, resolving_move_twice_throws) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    std::thread resolveThread([&player]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        player.resolveCurrentMovePromise("e2e4");
        ASSERT_ANY_THROW(player.resolveCurrentMovePromise("d2d4"));
    });

    auto result = player.makeResult(game, std::chrono::milliseconds(100));
    resolveThread.join();
};

TEST(AsynchronousPlayer, resolving_promise_with_illegal_move_throws) {
    SpellChessLib::AsynchronousPlayer player;
    SpellChessLib::Game game = gameFixture();

    auto result = player.makeResult(game, std::chrono::milliseconds(100));
    ASSERT_ANY_THROW(player.resolveCurrentMovePromise("e7e5"));
};