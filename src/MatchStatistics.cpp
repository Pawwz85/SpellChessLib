#include <SpellChess/MatchStatistics.h>
#include <cmath>
#include <iostream>

double SpellChessLib::Stats::getScore(const TrinomialModel& model) {
	return model.wins + double(model.draws) / 2;
}

double SpellChessLib::Stats::winRatio(const TrinomialModel& model){
	auto n = getNumberOfGames(model);
	if (n == 0)
		throw EmptyModelError();
	return double(model.wins) / n;
}

double SpellChessLib::Stats::drawRatio(const TrinomialModel& model) {
	auto n = getNumberOfGames(model);
	if (n == 0)
		throw EmptyModelError();
	return double(model.draws) / n;
}

unsigned int SpellChessLib::Stats::getNumberOfGames(const TrinomialModel& model) {
	return model.draws + model.losses + model.wins;
}

double SpellChessLib::Stats::eloDiff(const TrinomialModel& model){
	double expectedScore = winRatio(model) + drawRatio(model)/2;
	double epsilon = 1e-5; // the lost of accuracy based on that epsilon is about +- 0.01 elo. We could choose lower epsilon, but we will lose -2000l; 2000 range property 
	double delta = 400 * log10(expectedScore / (1 - expectedScore + epsilon) + epsilon);
	return delta;
}

double SpellChessLib::Stats::winProbability(double deltaElo){
	double epsilon = 1e-5;
	double tmp = std::pow( 10, (deltaElo / 400));
	return tmp / (1 + tmp);
}

double SpellChessLib::Stats::calculateLLR(const TrinomialModel& model, const double& elo0, const double& elo1){

	unsigned int N = getNumberOfGames(model);
	if (N == 0)
		throw EmptyModelError();

	unsigned int score = 2 * model.wins + model.draws;
	double P0 = winProbability(elo0) ;
	double P1 = winProbability(elo1);
	double n_s = score / double(2 * N); // normalized score
	double LLR = N * (n_s * log(P1 / P0) + (1 - n_s) * log((1 - P1) / (1 - P0)));
	return LLR;
}

double SpellChessLib::Stats::calculateLLRLowerThreshold(double alpha, double beta){
	return log(beta / (1 - alpha));
}

double SpellChessLib::Stats::calculateLLRUpperThreshold(double alpha, double beta) {
	return log((1 - beta) / alpha);
}

void SpellChessLib::Stats::PlayerOutcomeTable::init(int playerId)
{
	this->playerId = playerId;
	label = "Player " + std::to_string(playerId);
	for (size_t i = 0; i < GamesStatusCode_membersCount; ++i)
		wins[i] = loses[i] = draws[i] = 0;
}


void SpellChessLib::Stats::PlayerOutcomeTable::display() const
{
	GameStatusCode;
	
	std::cout << std::endl << label << " - wins:\n";
	std::cout << "Wins by checkmate: " << wins[GameStatusCode::CheckMate] << "\n";
	std::cout << "Wins on time: " << wins[GameStatusCode::OpponentTimedOut] << "\n";
	std::cout << "Wins by opponent playing illegal move: " << wins[GameStatusCode::IllegalMovePlayed] << "\n";
	std::cout << "Wins by capturing enemy king: " << wins[GameStatusCode::KingCaptured] << "\n";
	std::cout << "Wins by opponent disconnecting: " << wins[GameStatusCode::OpponentDisconnected] << "\n";
	std::cout << "Wins by opponent crashing: " << wins[GameStatusCode::PlayerCrashed] << "\n";

	std::cout << std::endl << label << " - draws:\n";
	std::cout << "Stalemate: " << draws[GameStatusCode::Stalemate] << "\n";
	std::cout << "50 moves rule: " << draws[GameStatusCode::DrawBy50MoveRule] << "\n";
	std::cout << "Insufficient material: " << draws[GameStatusCode::InsufficientMaterial] << "\n";
	std::cout << "Draws on timeout" << draws[GameStatusCode::OpponentTimedOutWithoutSufficientMatingMaterial] << "\n";

	std::cout << std::endl << label << " - loses:\n";
	std::cout << "Got checkmated: " << loses[GameStatusCode::CheckMate] << "\n";
	std::cout << "Lost on time: " << loses[GameStatusCode::OpponentTimedOut] << "\n";
	std::cout << "Played an illegal move: " << loses[GameStatusCode::IllegalMovePlayed] << "\n";
	std::cout << "Loses by king capture: " << loses[GameStatusCode::KingCaptured] << "\n";
	std::cout << "Loses by disconnection: " << loses[GameStatusCode::OpponentDisconnected] << "\n";
	std::cout << "Loses by crashing: " << loses[GameStatusCode::PlayerCrashed] << "\n";
}

void SpellChessLib::Stats::MatchOutcomeTables::init(const std::vector<int>& playerIds)
{
	this->playerTables.resize(playerIds.size());

	for (size_t i = 0; i < playerIds.size(); ++i)
		playerTables[i].init(playerIds[i]);
}

void SpellChessLib::Stats::MatchOutcomeTables::registerOutcome(int whitePlayerId, int blackPlayerId, PlayerColor winner, GameStatusCode code)
{
	
	bool isWinner, isLoser, isDraw;

	for (auto& playerTable : playerTables) {
		isWinner = isLoser = isDraw = false;
		if (playerTable.playerId == whitePlayerId) {
			isWinner = winner == White;
			isLoser  = winner == Black;
			isDraw = winner == Nobody;
		}

		if (playerTable.playerId == blackPlayerId) {
			isWinner = winner == Black;
			isLoser = winner == White;
			isDraw = winner == Nobody;
		}

		if (isWinner)
			playerTable.wins[code]++;
		if (isLoser)
			playerTable.loses[code]++;
		if (isDraw)
			playerTable.draws[code]++;
	}


}

void SpellChessLib::Stats::MatchOutcomeTables::display() const
{
	for (auto& table : playerTables)
		table.display();
}
