#include <SpellChess/SPRTTournament.h>
#include <SpellChess/PlayerRegistry.h>
#include <iostream>
#include <cstdlib>

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

    return GameRunnerPool::GameRunRequest();
}

void SpellChessLib::SPRTPTournament::displayStats() const
{
    std::cout << "\033[2J\033[1;1H";
    std::cout << "Total games = " << Stats::getNumberOfGames(tournamentModel) << '\n';
    std::cout << "Win ratio = " << Stats::winRatio(tournamentModel) << '\n';
    std::cout << "Draw ratio = " << Stats::drawRatio(tournamentModel) << '\n';
    std::cout << "Elo difference = " << Stats::eloDiff(tournamentModel) << "\n";
    std::cout << "LLR  = " << Stats::calculateLLR(tournamentModel, elo0, elo1) << "\n";
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
    displayStats();


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

    auto annotations = game.getAnnotations();
    annotations.tagSection["Round"] = "?";
    annotations.tagSection["Result"] = Annotation::makeResultTag(game.getAnnotations().terminationReason);

    std::string saveLocation = "Games/game_" + std::to_string(Stats::getNumberOfGames(tournamentModel));
    Annotation::exportAnnotatedGame(saveLocation, annotations);
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
}

void SpellChessLib::SPRTPTournament::startTest(int measuredPlayer, int opponent){
    this->measuredPlayer = measuredPlayer;
    this->opponent = opponent;

    outcomeTables.init(std::vector<int>({ measuredPlayer, opponent }));

    for (size_t i = 0; i < runners->getConcurrentGamesValue(); ++i)
        this->spawnNewGameRequest();
}

SpellChessLib::Stats::TrinomialModel SpellChessLib::SPRTPTournament::waitForCompletion() {
    {
        std::unique_lock<std::mutex> guard(lock);
        con_var.wait(guard, [this]() {return this->SPRTCompleted; });
        
        displayStats();
        outcomeTables.display();

        return tournamentModel;
    }
}

SpellChessLib::GameState SpellChessLib::RandomPositionSupplier::recursiveRandomization(GameState in, int level)
{
    if (level == 0)
        return in;

    MoveGenerator gen1(in);
    auto moves = gen1.generateMoves();
    size_t index;

    GameState out;
    bool isInvalid;
    do {
        do {
            index = rand() % moves.size();
        } while (moves[index].spellPolicy != UserCanAddSpell);

        isInvalid = false;
        out = in;
        out.makeMove(moves[index].move);
        out = recursiveRandomization(out, level - 1);
        
        MoveGenerator gen2(out);
        if (gen2.generateMoves().empty())
            isInvalid = true;

    } while (isInvalid);

    return out;
}
