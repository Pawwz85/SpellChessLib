#pragma once

#include <stdexcept>
#include "DataTables.h"
#include "SpellChessGame.h"

namespace SpellChessLib {

	namespace Stats {

		struct TrinomialModel {
			unsigned int wins;
			unsigned int draws;
			unsigned int losses;
		};

		enum ELOType {
			Logistic,

		};

		class EmptyModelError : public std::exception {
		public:
			const char* what() const noexcept override {
				return "Certain statistical functions are not available for empty models";
			}
		};

		struct PlayerOutcomeTable {
			int playerId;
			std::string label = "";

			unsigned int wins[GamesStatusCode_membersCount];
			unsigned int loses[GamesStatusCode_membersCount];
			unsigned int draws[GamesStatusCode_membersCount];

			void init(int playerId);
			void display() const;
		};
		
		struct MatchOutcomeTables {
			std::vector<PlayerOutcomeTable> playerTables;

			void init(const std::vector<int> & playerIds);
			void registerOutcome(int whitePlayerId, int blackPlayerId, PlayerColor winner, GameStatusCode code);
			void display() const;
		};
		


		/*!
			Returns the total score achieved by this particular player.
		*/
		extern double getScore(const TrinomialModel& model);

		/*!
			Returns the win ratio achieved by this particular player. Throws EmptyModelError if empty model was given
		*/
		extern double winRatio(const TrinomialModel & model);

		/*!
			Returns the draw ratio achieved by this particular player. Throws EmptyModelError if empty model was given
		*/
		extern double drawRatio(const TrinomialModel& model);
		
		/*!
			Get the total number of games played by this particular player
		*/
		extern unsigned int getNumberOfGames(const TrinomialModel& model);

		

		/*!
			Returns the elo difference to an average opponent. The resulting difference is clipped to [-2000; 2000] elo range to prevent nan results 
			Throws EmptyModelError if empty model was given
		*/
		extern double eloDiff(const TrinomialModel& model);

		/* 
			Assuming elo_diff is elo difference calculated by eloDiff function, this function will return such k
			that interval [elo_diff - k, elo_diff + k] will contain real elo difference with the probability 1 - alpha
		*/
		extern double eloDiffConfidenceInterval(const TrinomialModel& model, double alpha = 0.05);

		/*!
			Returns the expected win probability based on the elo difference to an opponent
		*/
		extern double winProbability(double deltaElo);

		/*
			Calculates the logarithm of sequential probability ratio. The model parameter stores the current WDL (wins, draws and loses) values of our test.
			The elo0 parameter represents the null hypothesis of difference of this player to his average opponent, while elo1 represents the alternative hypothesis.
			The returned value is a logarithm of P ( model | elo0) / P (model | elo1), where P (model | elox) represents the probability of observing results stored in the model
			assuming hypothesis elox is correct.
		*/
		extern double calculateLLR(const TrinomialModel& model, const double & elo0, const double & elo1);

		/*!
			Calculate lower LLR threshold. If calculateLLR returns value lower than that, we should accept the null hypothesis about elo difference
		*/
		extern double calculateLLRLowerThreshold(double alpha, double beta);

		/*!
			Calculate upper LLR threshold.  If calculateLLR returns value higher than that, we should accept the null hypothesis about elo difference
		*/
		extern double calculateLLRUpperThreshold(double alpha, double beta);
	}

}