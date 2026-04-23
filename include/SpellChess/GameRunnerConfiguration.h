#pragma once

#include "SpellChessGame.h"
#include <vector>
#include <memory>

namespace SpellChessLib {

	struct GameRunnerConfiguration {
		enum TimerType {
			INFINITE,
			CLASSICAL
		};

		struct ClassicalTimerData {
			uint32_t base;
			uint32_t inc;
		};

		union TimerData {
			std::nullptr_t infiniteTimerData;
			ClassicalTimerData classicalTimerData;
		};

		struct TimerInfo {
			TimerType type;
			TimerData data;
		};

		struct EngineStopConditions {
			uint32_t moveTime;
			uint32_t nodes;
			uint32_t depth;
		};


		std::string rootFen;
		std::vector<SpellChessMove> rootMoves;
		bool allowPondering;

		TimerInfo whPlayerTimerInfo;
		TimerInfo blPlayerTimerInfo;
		EngineStopConditions stopConditions;

		static std::unique_ptr<TimerController> createTimerFromInfo(std::shared_ptr<TimerService> timerService, const TimerInfo & info);
		
		std::string event;
		std::string site;
		std::string round;
	};

	class GameRunnerConfigurationBuilder {
		GameRunnerConfiguration config;
	public:

		enum PlayerColor {
			All,
			White,
			Black
		};

		GameRunnerConfigurationBuilder() : config({ START_POS, {}, false, { GameRunnerConfiguration::INFINITE, nullptr }, { GameRunnerConfiguration::INFINITE, nullptr }, {0, 0, 0}, "?", "?", "?"}) {};

		GameRunnerConfigurationBuilder& withPosition(const std::string& fen, const std::vector<SpellChessMove>& moves = {});
		GameRunnerConfigurationBuilder& withPondering(bool allowPondering = true);
		GameRunnerConfigurationBuilder& withInfiniteTime(GameRunnerConfigurationBuilder::PlayerColor color = All);
		GameRunnerConfigurationBuilder& withClassicalTimer( const uint32_t& base, const uint32_t& inc, GameRunnerConfigurationBuilder::PlayerColor color = All);
		GameRunnerConfigurationBuilder& withNodeLimit(const uint32_t& value);
		GameRunnerConfigurationBuilder& withDepthLimit(const uint32_t& value);
		GameRunnerConfigurationBuilder& withMoveTimeLimit(const uint32_t& value);
		GameRunnerConfigurationBuilder& withEvent(const std::string& value);
		GameRunnerConfigurationBuilder& withSite(const std::string& value);
		GameRunnerConfigurationBuilder& withRound(const std::string& value);
		GameRunnerConfiguration build() { return config; };
	};
}
