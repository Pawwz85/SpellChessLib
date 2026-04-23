#include <SpellChess/SpellChessGame.h>
#include <SpellChess/MoveLegality.h>

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

void SpellChessLib::Game::setOutcome(const PlayerColor& winner, const GameStatusCode& outcome)
{
	std::lock_guard<std::mutex> guard(lock);
	
	if (statusCode != StillOngoing)
		return; // do not allow overriding outcome

	this->winner = winner;
	this->statusCode = outcome;

	if (winner == White)
		annotations.terminationReason = Annotation::WhiteWon;
	else if (winner == Black)
		annotations.terminationReason = Annotation::BlackWon;
	else
		annotations.terminationReason = Annotation::Draw;

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

void SpellChessLib::Game::makeMove(const SpellChessMove& move) {
	auto movingSide = state.getSideToMove();
	if (!MoveGenerator(state).isMoveLegal(move)) {
		setOutcome(flipColor(movingSide), IllegalMovePlayed);
		annotations.annotatedSequence.pushBack(Annotation::makeIllegalMoveComment(movingSide, move)); // write illegal move comment to the sequence
		return;
	}
	auto & movingSideTimer = (movingSide == White) ? whTimer : blTimer;
	auto& opponentTimer = (movingSide == White) ? blTimer : whTimer;

	movingSideTimer->onMovePlayed();
	
	Piece capturedPiece = state.getSquare(move.to).piece;
	state.makeMove(move);
	movesSinceRoot.push_back(move);
	annotations.annotatedSequence.pushBack(move);

	if (capturedPiece.owner != Nobody && capturedPiece.type == King) {
		setOutcome(movingSide, KingCaptured);
		return;
	}
	
	if (!state.hasSufficientMatingMaterial(White) && !state.hasSufficientMatingMaterial(Black)) {
		setOutcome(Nobody, InsufficientMaterial);
		return;
	}

	MoveGenerator gen(state);
	auto moves = gen.generateMoves();

	if (state.getHalfMoveCounter() >= 100) {
		setOutcome(Nobody, DrawBy50MoveRule);
		return;
	}

	if (moves.empty()) {
		if (gen.isInCheck())
			setOutcome(movingSide, CheckMate);
		else
			setOutcome(Nobody, Stalemate);
		return;
	}

	opponentTimer->resume();
	// todo: 3-fold repetition 
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
	 Annotation::TagSection::const_iterator it;

	 it = annotations.tagSection.find("Event");
	 result.event = (it == annotations.tagSection.end()) ? "?" : it->value;
	 it = annotations.tagSection.find("Site");
	 result.site = (it == annotations.tagSection.end()) ? "?" : it->value;
	 it = annotations.tagSection.find("Date");
	 result.date = (it == annotations.tagSection.end()) ? "????.??.??" : it->value;
	 it = annotations.tagSection.find("Round");
	 result.round = (it == annotations.tagSection.end()) ? "*" : it->value;
	 it = annotations.tagSection.find("White");
	 result.white = (it == annotations.tagSection.end()) ? "?" : it->value;
	 it = annotations.tagSection.find("Black");
	 result.black = (it == annotations.tagSection.end()) ? "?" : it->value;
	 it = annotations.tagSection.find("Result");
	 result.result = (it == annotations.tagSection.end()) ? "*" : it->value;
	 return result;
}

UCILoader::GoParams<SpellChessLib::SpellChessMove> SpellChessLib::Game::getGoParams() const {
	exportGoParams(goParamsBuilder);
	return goParamsBuilder.build();
}

void SpellChessLib::Game::setSevenTagRooster(const Annotation::SevenTagRoster& roster){
	annotations.tagSection["Event"] = roster.event;
	annotations.tagSection["Site"]  = roster.site;
	annotations.tagSection["Date"]  = roster.date;
	annotations.tagSection["Round"] = roster.round;
	annotations.tagSection["White"] = roster.white;
	annotations.tagSection["Black"] = roster.black;
	annotations.tagSection["Result"] = roster.result;
}

const SpellChessLib::Annotation::GameAnnotations& SpellChessLib::Game::getAnnotations() const{
	return annotations;
}

void SpellChessLib::Game::exportGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& target) const{
	whTimer->exportAsGoParams(target, White);
	blTimer->exportAsGoParams(target, Black);
}
