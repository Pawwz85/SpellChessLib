#include <SpellChess/SPRTTournament.h>
#include <SpellChess/PlayerRegistry.h>
#include <SpellChess/GameAnnotations.h>
#include <SpellChess/GameRunnerPool.h>
#include <SpellChess/MatchStatistics.h>
#include <SpellChess/PlayerEntry.h>
#include <SpellChess/PositionSupplier.h>
#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/SpellChessGame.h>
#include <SpellChess/GameRunnerConfiguration.h>
#include <SpellChess/Board.h>
#include <iostream>
#include <exception>
#include <functional>
#include <string>
#include <utility>
#include <vector>

SpellChessLib::GameRunnerPool::GameRunRequest SpellChessLib::SPRTPTournament::makeRequest(int whitePlayerId, int blackPlayerId, const GameState& root)
{
    PlayerEntry whitePlayer = PlayerRegistry->get(whitePlayerId);
    PlayerEntry blackPlayer = PlayerRegistry->get(blackPlayerId);

    return {
        whitePlayer,
        blackPlayer,
        configBuilder.withPosition(root.toFen()).build(),
        nullptr, // game monitor. Todo : allow attaching one
        false, // do not ignore preferences
        [this](const Game& game, const GameRunnerPool::GameRunRequest& request) {this->onGameCompleted(game, request); },
         [this](const GameRunnerPool::GameRunRequest& request, const std::exception & _) {this->onGameCrashed(request); }
    };

}

void SpellChessLib::SPRTPTournament::displayStats(const SpellChessLib::SPRTStatiscs & stats) const
{
    if (!shouldDisplayStats)
        return;

    std::cout << "\033[2J\033[1;1H";
    std::cout << "Total games =" << stats.totalGames << '\n';
    std::cout << "About ~ " << stats.gamesLeftEstimate <<" games left\n";
    std::cout << "Win ratio = " << stats.winRatio << '\n';
    std::cout << "Draw ratio = " << stats.drawRatio << '\n';
    std::cout << "Elo difference = " << stats.eloDiff << " +- " << stats.eloDiffConfidenceInterval <<"\n";
    std::cout << "LLR  = " << stats.LLR << "\n";
}

void SpellChessLib::SPRTPTournament::updateModel(const PlayerColor& winner, const GameRunnerPool::GameRunRequest& request){

    if (winner == Nobody) {
        tournamentModel.draws++;
        return;
    }


    PlayerColor measuredPlayerColor = (request.whPlayer.id == measuredPlayer) ? White: Black;

    if (winner == measuredPlayerColor)
        tournamentModel.wins++;
    else
        tournamentModel.losses++;

}

void SpellChessLib::SPRTPTournament::onGameCompleted(const Game& game, const GameRunnerPool::GameRunRequest& request){
    std::unique_lock<std::mutex> guard(lock);
    --gamesIssued;

    outcomeTables.registerOutcome(request.whPlayer.id, request.blPlayer.id, game.getWinner(), game.getStatusCode());
    
    updateModel(game.getWinner(), request);
    auto stats = calculateStats();
    onStatsUpdate(stats);
    displayStats(stats);

    double LLR = Stats::calculateLLR(tournamentModel, elo0, elo1);
    if (LLR < Stats::calculateLLRLowerThreshold(alpha, beta)) {
        SPRTCompleted = true;
    }

    if (LLR > Stats::calculateLLRUpperThreshold(alpha, beta)) {
        SPRTCompleted = true;
    }

    if (!SPRTCompleted)
        spawnNewGameRequest();

    con_var.notify_one();

    auto annotations = game.getDocument()->toAnnotations();

    std::filesystem::path saveLocation = gameSaveLocation / ("game_" + std::to_string(Stats::getNumberOfGames(tournamentModel)));
    std::string saveLocationString = saveLocation.string();
    Annotation::exportAnnotatedGame(saveLocationString, annotations);
}

void SpellChessLib::SPRTPTournament::onGameCrashed(const GameRunnerPool::GameRunRequest& request){
    std::lock_guard<std::mutex> guard(lock);

    if (!SPRTCompleted) 
        spawnNewGameRequest();
}

void SpellChessLib::SPRTPTournament::spawnNewGameRequest(){

    int whitePlayer, blackPlayer;

    if (gameCounter % 2) {
        whitePlayer = measuredPlayer;
        blackPlayer = opponent;
    }
    else {
        whitePlayer = opponent;
        blackPlayer = measuredPlayer;
    }

    runners->addRequest(std::move(makeRequest(whitePlayer, blackPlayer, supplier->produce())));
    ++gameCounter;
    ++gamesIssued;
}

SpellChessLib::SPRTStatiscs 
SpellChessLib::SPRTPTournament::calculateStats() const {
    SPRTStatiscs stats;
    stats.totalGames = Stats::getNumberOfGames(tournamentModel);
    stats.gamesLeftEstimate = (unsigned int) Stats::estimateGamesLeft(tournamentModel, elo0, elo1, alpha, beta);
    stats.winRatio = Stats::winRatio(tournamentModel);
    stats.drawRatio = Stats::drawRatio(tournamentModel);
    stats.eloDiff = Stats::eloDiff(tournamentModel);
    stats.eloDiffConfidenceInterval = Stats::eloDiffConfidenceInterval(tournamentModel);
    stats.LLR = Stats::calculateLLR(tournamentModel, elo0, elo1);
    return stats;
}


void SpellChessLib::SPRTPTournament::startTest(int measuredPlayer, int opponent){
    this->measuredPlayer = measuredPlayer;
    this->opponent = opponent;

    outcomeTables.init(std::vector<int>({ measuredPlayer, opponent }));

    for (size_t i = 0; i < runners->getConcurrentGamesValue(); ++i)
        this->spawnNewGameRequest();
}

void SpellChessLib::SPRTPTournament::setMoveTime(unsigned int moveTimeMs) {
    configBuilder.withMoveTimeLimit(moveTimeMs);
}

void SpellChessLib::SPRTPTournament::setTimeFormat(unsigned int baseTimeMs, unsigned int incMs, unsigned int margin) {
    configBuilder.withClassicalTimer(baseTimeMs, incMs, margin);
}

void SpellChessLib::SPRTPTournament::setOnStatsUpdate(std::function<void(SPRTStatiscs)> callback) {
    this->onStatsUpdate = callback;
};

void SpellChessLib::SPRTPTournament::setGameSaveLocation(const std::filesystem::path & path) {
    this->gameSaveLocation = path;
}

SpellChessLib::Stats::TrinomialModel SpellChessLib::SPRTPTournament::waitForCompletion() {
    {
        std::unique_lock<std::mutex> guard(lock);
        con_var.wait(guard, [this]() {return this->SPRTCompleted;});
        
        displayStats(calculateStats());
        outcomeTables.display();

        return tournamentModel;
    }
}


