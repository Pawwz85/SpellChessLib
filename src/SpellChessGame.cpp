#include <SpellChess/SpellChessGame.h>
#include <SpellChess/MoveLegality.h>
#include <cassert>
#include <stdexcept>

void SpellChessLib::Game::notifyResultObservers(){
	for(auto & weakPointer : resultObservers)
		if (auto resultObserver = weakPointer.lock()) 
			resultObserver->onGameOutcome(winner, statusCode);
}

void SpellChessLib::Game::addResultObserver(std::weak_ptr<ResultObserver> observer){
	resultObservers.push_back(observer);
}

void SpellChessLib::Game::timeOutPlayer(const PlayerColor& color){
    if (state.hasSufficientMatingMaterial(flipColor(color)))
		setOutcome(flipColor(color), OpponentTimedOut);
	else
		setOutcome(Nobody, OpponentTimedOutWithoutSufficientMatingMaterial);
}

inline void appendCommentToGameDocument(SpellChessLib::SpellChessDocument & document, 
	SpellChessLib::SpellChessDocument::Handle where,
	const std::string & comment) {
		std::string finalComment = document.getComment(where);
		if (!finalComment.empty()) 
			finalComment += " ";
		finalComment += comment;
		document.setComment(where, finalComment);
	};

void SpellChessLib::Game::writeComment(const std::string & comment) {
	appendCommentToGameDocument(*gameDocument, gameDocument->getTail(), comment);
};

void SpellChessLib::Game::setOutcome(const PlayerColor& winner, const GameStatusCode& outcome)
{
	std::lock_guard<std::mutex> guard(lock);
	
	if (statusCode != StillOngoing)
		return; // do not allow overriding outcome

	this->winner = winner;
	this->statusCode = outcome;

	whTimer->pause();
	blTimer->pause();

	if (winner == White)
		gameDocument->setGameResult(Annotation::WhiteWon);
	else if (winner == Black)
		gameDocument->setGameResult(Annotation::BlackWon);
	else
		gameDocument->setGameResult(Annotation::Draw);

	writeComment(make_result_comment(winner, outcome));

	notifyResultObservers();
}

const SpellChessLib::GameState& SpellChessLib::Game::getRoot() {
	return root; 
}


#define THREAD_SAFE_GETTER(member) std::lock_guard<std::mutex> guard(lock); return member

std::vector<SpellChessLib::SpellChessMove> SpellChessLib::Game::getMovesSinceRoot() const {
	THREAD_SAFE_GETTER(movesSinceRoot);
}
SpellChessLib::GameState SpellChessLib::Game::getRoot() const{
	THREAD_SAFE_GETTER(root);
}
SpellChessLib::GameState SpellChessLib::Game::getGameState() const {
	THREAD_SAFE_GETTER(state);
}

SpellChessLib::PlayerColor SpellChessLib::Game::getWinner() const {
	THREAD_SAFE_GETTER(winner);
}

SpellChessLib::GameStatusCode SpellChessLib::Game::getStatusCode() const {
	THREAD_SAFE_GETTER(statusCode);
}
#undef THREAD_SAFE_GETTER

std::chrono::milliseconds SpellChessLib::Game::getRemainingTime(const PlayerColor& color) const {
	return color == White ? whTimer->getValue() : blTimer->getValue();
}

size_t SpellChessLib::Game::countOccurences(const GameState & position) const {
	uint64_t hash = calculateHash(position);
	return std::count(posHistory.begin(), posHistory.end(), hash);
};

std::string SpellChessLib::Game::make_result_comment(const PlayerColor &winner, const GameStatusCode &outcome){

	static std::map<PlayerColor, std::string> prefix = {
		{Nobody, "Draw "},
		{White,  "White won "},
		{Black,  "Black won "},
	};

	static std::map<GameStatusCode, std::string> reason = {
		{StillOngoing, "by cheating or miracle"},	// illegal reason
		{IllegalMovePlayed, "because opponent tried playing illegal move"},
		{CheckMate, "by checkmate"},
		{Stalemate, "by stalemate"},
		{InsufficientMaterial, "by insufficient material"},
		{OpponentTimedOutWithoutSufficientMatingMaterial, "by timeout"},
		{OpponentTimedOut, "by timeout"},
		{DrawBy50MoveRule, "because of 50 move rule"},
		{DrawBy3FoldRepetition, "by threefold repetition"},
		{OpponentDisconnected, "bacause opponent disconected"},
		{KingCaptured, "because opponent king was captured"},
		{PlayerCrashed, "because opponent threw an error"},
		{GamesStatusCode_membersCount, "by cheating or miracle"} // illegal reason
	};

    return prefix[winner] + reason[outcome];
}

SpellChessLib::GameStatusCode gameStatusCodeFromArbiterVerdict(SpellChessLib::ArbiterVerdict verdict) {
	switch (verdict)
	{
	case SpellChessLib::ArbiterVerdict::None:
		return SpellChessLib::GameStatusCode::StillOngoing;
	case SpellChessLib::ArbiterVerdict::Checkmate:
		return SpellChessLib::GameStatusCode::CheckMate;
	case SpellChessLib::ArbiterVerdict::KingCapture:
		return SpellChessLib::GameStatusCode::KingCaptured;
	case SpellChessLib::ArbiterVerdict::Stalemate:
		return SpellChessLib::GameStatusCode::Stalemate;
	case SpellChessLib::ArbiterVerdict::DrawByFiftyMoveRule:
		return SpellChessLib::GameStatusCode::DrawBy50MoveRule;
	case SpellChessLib::ArbiterVerdict::DrawByInsufficientMaterial:
		return SpellChessLib::GameStatusCode::InsufficientMaterial;
	case SpellChessLib::ArbiterVerdict::ThreefoldRepetition:
		return SpellChessLib::GameStatusCode::DrawBy3FoldRepetition;
	}

	throw std::logic_error("Unrecognised arbiter verdict");
}


void SpellChessLib::Game::makeMove(const SpellChessMove& move) {
	auto movingSide = state.getSideToMove();
	if (!MoveGenerator(state).isMoveLegal(move)) {
		setOutcome(flipColor(movingSide), IllegalMovePlayed);
		writeComment(Annotation::makeIllegalMoveComment(movingSide, move));
		return;
	}
	auto & movingSideTimer = (movingSide == White) ? whTimer : blTimer;
	auto& opponentTimer = (movingSide == White) ? blTimer : whTimer;

	movingSideTimer->onMovePlayed();
	
	Piece capturedPiece = state.getSquare(move.to).piece;
	state.makeMove(move);

	if (state.getHalfMoveCounter() == 0)
		posHistory.clear(); // clear pos history

	movesSinceRoot.push_back(move);
	posHistory.push_back(calculateHash(state));

	gameDocument->makeMove(gameDocument->getTail(), move);

	ArbiterVerdict verdict = arbite(state, *this);

	if (verdict != ArbiterVerdict::None) {
		PlayerColor winner = (verdict == ArbiterVerdict::Checkmate || verdict == ArbiterVerdict::KingCapture)?
			movingSide : Nobody;
		setOutcome(winner, gameStatusCodeFromArbiterVerdict(verdict));
		return;	
	}

	opponentTimer->resume();
}

void SpellChessLib::Game::startCurrentPlayerTimer(){
	(state.getSideToMove() == White ? whTimer : blTimer)->resume();
}

void SpellChessLib::Game::resign(const PlayerColor& side, const ResignReason& reason)
{
	switch (reason)
	{
	case ExceptionThrownByAPlayer:
		setOutcome(flipColor(side), PlayerCrashed);
		break;
	default:
		break;
	}
}

UCILoader::GoParamsBuilder<SpellChessLib::SpellChessMove>& SpellChessLib::Game::getGoParamsBuilder(){
	return goParamsBuilder;
}

SpellChessLib::Annotation::SevenTagRoster SpellChessLib::Game::getRoster() const
{
	 Annotation::SevenTagRoster result;
	 
	 result.event = gameDocument->getTag("Event").value_or("?");
	 result.site = gameDocument->getTag("Site").value_or("?");
	 result.date = gameDocument->getTag("Date").value_or("??.??.????");
	 result.round = gameDocument->getTag("Round").value_or("?");
	 result.white = gameDocument->getTag("White").value_or("?");
	 result.black = gameDocument->getTag("Black").value_or("?");
	 result.result = gameDocument->getTag("Result").value_or("*");

	 return result;
}

UCILoader::GoParams<SpellChessLib::SpellChessMove> SpellChessLib::Game::getGoParams() const {
	exportGoParams(goParamsBuilder);
	return goParamsBuilder.build();
}

void SpellChessLib::Game::setSevenTagRooster(const Annotation::SevenTagRoster& roster){
	gameDocument->setTag("Event", roster.event);
	gameDocument->setTag("Site", roster.site);
	gameDocument->setTag("Date", roster.date);
	gameDocument->setTag("Round", roster.round);
	gameDocument->setTag("White", roster.white);
	gameDocument->setTag("Black", roster.black);
}

void SpellChessLib::Game::exportGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& target) const{
	whTimer->exportAsGoParams(target, White);
	blTimer->exportAsGoParams(target, Black);
}

SpellChessLib::TimerPairState SpellChessLib::Game::getTimerPair() const {
	return TimerPairState{whTimer->exportState(), blTimer->exportState()};
}

std::shared_ptr<SpellChessLib::SpellChessDocument>
SpellChessLib::Game::getDocument() {
	return gameDocument;
};

std::shared_ptr<const SpellChessLib::SpellChessDocument>
SpellChessLib::Game::getDocument() const {
	return gameDocument;
};