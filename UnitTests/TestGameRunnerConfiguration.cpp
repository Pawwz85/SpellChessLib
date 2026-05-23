#include "pch.h"

#include <SpellChess/GameRunnerConfiguration.h>
#include "utils.h"

using namespace SpellChessLib;

TEST(GameRunnerConfigurationBuilder, defaultValues) {
	GameRunnerConfigurationBuilder builder;
	auto config = builder.build();

	ASSERT_EQ(config.rootFen, START_POS);
	ASSERT_EQ(config.rootMoves.size(), 0);
	ASSERT_FALSE(config.allowPondering);
	ASSERT_EQ(config.whPlayerTimerInfo.type, GameRunnerConfiguration::INFINITE);
	ASSERT_EQ(config.blPlayerTimerInfo.type, GameRunnerConfiguration::INFINITE);
}

TEST(GameRunnerConfigurationBuilder, settingStartPosAndPondering) {
	GameRunnerConfigurationBuilder builder;
	auto config = builder.withPosition("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2", parseMoveList({ "g1f3" }))
			.withPondering()
			.build();

	ASSERT_EQ(config.rootFen, "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2");
	ASSERT_EQ(config.rootMoves.size(), 1);
	SpellChessMove testMove = parseMove("g1f3");
	ASSERT_TRUE(std::memcmp(&config.rootMoves.at(0), &testMove, sizeof(SpellChessMove)) == 0);
	ASSERT_TRUE(config.allowPondering);
}

TEST(GameRunnerConfigurationBuilder, setClassicalTimeControlForWhite) {
	GameRunnerConfigurationBuilder builder;
	auto config = builder.withClassicalTimer(100, 10, 5, GameRunnerConfigurationBuilder::White).build();
	ASSERT_EQ(config.whPlayerTimerInfo.type, GameRunnerConfiguration::CLASSICAL);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.base, 100);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.inc, 10);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.margin, 5);
	ASSERT_EQ(config.blPlayerTimerInfo.type, GameRunnerConfiguration::INFINITE);
}

TEST(GameRunnerConfigurationBuilder, setClassicalTimeControlForBlack) {
	GameRunnerConfigurationBuilder builder;
	auto config = builder.withClassicalTimer(100, 10, 5, GameRunnerConfigurationBuilder::Black).build();
	ASSERT_EQ(config.blPlayerTimerInfo.type, GameRunnerConfiguration::CLASSICAL);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.base, 100);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.inc, 10);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.margin, 5);
	ASSERT_EQ(config.whPlayerTimerInfo.type, GameRunnerConfiguration::INFINITE);
}

TEST(GameRunnerConfigurationBuilder, setClassicalTimeControlForAll) {
	GameRunnerConfigurationBuilder builder;
	auto config = builder.withClassicalTimer(100, 10, 5).build();
	ASSERT_EQ(config.whPlayerTimerInfo.type, GameRunnerConfiguration::CLASSICAL);
	ASSERT_EQ(config.blPlayerTimerInfo.type, GameRunnerConfiguration::CLASSICAL);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.base, 100);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.inc, 10);
	ASSERT_EQ(config.whPlayerTimerInfo.data.classicalTimerData.margin, 5);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.base, 100);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.inc, 10);
	ASSERT_EQ(config.blPlayerTimerInfo.data.classicalTimerData.margin, 5);
}
