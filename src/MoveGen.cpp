#include <SpellChess/MoveGen.h>
#include <SpellChess/MoveLegality.h>
#include <SpellChess/Board.h>
#include <cstddef>
#include <cmath>

void SpellChessLib::MoveGenerator::resetCache()
{
	controlledByEnemy = 0;
	for (size_t i = 0; i < 64; ++i)
		pins[i] = NoSquare;
}

void SpellChessLib::MoveGenerator::cacheEnemyPawn(const SquareIndex& sq, const PlayerColor& us)
{
	int x = sq % 8, y = sq / 8;

	int dy = us == Black ? -1 : 1;
	if (x != 0)
		controlledByEnemy |= 1ull << squareIndexFromCords(x -1, y + dy);
	if (x != 7)
		controlledByEnemy |= 1ull << squareIndexFromCords(x + 1, y + dy);
}

#define _OFFSET_BOUNDARY_CHECK(dx, dy) if (CORDINATE_BOUNDARY_CHECK((x + dx)) && CORDINATE_BOUNDARY_CHECK((y + dy)))
#define _SET_BIT_BY_OFFSET(dx, dy) controlledByEnemy |= 1ull << squareIndexFromCords(x + dx, y + dy);
#define CHECK_OFFSET(dx, dy) _OFFSET_BOUNDARY_CHECK(dx, dy) _SET_BIT_BY_OFFSET(dx, dy)
void SpellChessLib::MoveGenerator::cacheEnemyKnight(const SquareIndex& sq)
{
	int x = sq % 8, y = sq / 8;
	CHECK_OFFSET(-2, -1)
	CHECK_OFFSET(-2, 1)
	CHECK_OFFSET(2, -1)
	CHECK_OFFSET(2, 1)
	CHECK_OFFSET(-1, -2)
	CHECK_OFFSET(-1, 2)
	CHECK_OFFSET(1, -2)
	CHECK_OFFSET(1, 2)
}

void SpellChessLib::MoveGenerator::cacheEnemyKing(const SquareIndex& sq)
{
	int x = sq % 8, y = sq / 8;
	CHECK_OFFSET(-1, -1)
	CHECK_OFFSET(-1, 0)
	CHECK_OFFSET(-1, 1)
	CHECK_OFFSET(0, -1)
	CHECK_OFFSET(0, 1)
	CHECK_OFFSET(1, -1)
	CHECK_OFFSET(1, 0)
	CHECK_OFFSET(1, 1)
}
#undef CHECK_OFFSET
#undef _SET_BIT_BY_OFFSET
#undef _OFFSET_BOUNDARY_CHECK

void SpellChessLib::MoveGenerator::cacheEnemySliderRay(const GameState & state, const SquareIndex& sq, const PlayerColor& us, const int& dx, const int& dy)
{
	bool pinUsed = false;
	bool kingEncountered = false;

	int x = sq % 8, y = sq / 8, i = 1;
	SquareIndex currSq = NoSquare;
	SquareIndex pinnedPieceCandidate = NoSquare;
	while (CORDINATE_BOUNDARY_CHECK(x + i * dx) && CORDINATE_BOUNDARY_CHECK(y + i * dy)) {
		currSq = squareIndexFromCords(x + i * dx, y + i * dy);

		if(!pinUsed || kingEncountered) 
			controlledByEnemy |= 1ull << currSq;

		if (state.getSquare(currSq).piece.type == King && state.getSquare(currSq).piece.owner == us) {
			pinUsed = true;
			kingEncountered = true;
			goto INCREMENT;
		}

		if (state.getSquare(currSq).isEmpty() || state.getJumpSquare() == currSq)
			goto INCREMENT;

		if (state.getSquare(currSq).piece.owner != us)
			break;

		if (pinUsed || kingEncountered)
			break;
		pinUsed = true;
		pinnedPieceCandidate = currSq;

		INCREMENT:
		++i;
	}

	if (kingEncountered && pinnedPieceCandidate != NoSquare) {
		pins[pinnedPieceCandidate] = sq;
	}

}

void SpellChessLib::MoveGenerator::cacheEnemySlider(const GameState & state, const SquareIndex& sq, const PlayerColor& us, const PieceType& sliderType)
{
	int ray_dxs[8] = {-1, -1,  1, 1, 1,-1, 0,  0};
	int ray_dys[8] = {-1,  1, -1, 1, 0, 0, 1, -1};
	int begin, end;

	switch (sliderType)
	{
	case Bishop:
		begin = 0;
		end = 4;
		break;
	case Rook:
		begin = 4;
		end = 8;
		break;
	case Queen:
		begin = 0;
		end = 8;
		break;
	default:
		return;
	}

	for (int i = begin; i < end; ++i)
		cacheEnemySliderRay(state, sq, us, ray_dxs[i], ray_dys[i]);

}

void SpellChessLib::MoveGenerator::calculateCache(const GameState& state)
{
	PlayerColor us = state.getSideToMove();
	PlayerColor enemy = flipColor(us);

	for (SquareIndex i = 0; i < 64; ++i) {
		const Piece& p = state.getSquare(i).piece;

		if (p == Piece({King, us})) {
			kingPos = i;
		}

		if (p.owner == enemy)
			switch (state.getSquare(i).piece.type) {
			case None: break;
			case Pawn:
				cacheEnemyPawn(i, us);
				break;
			case Knight:
				cacheEnemyKnight(i);
				break;
			case King:
				cacheEnemyKing(i);
				break;
			default:
				cacheEnemySlider(state, i, us, p.type);
				break;
			};
	}
	
	inCheck = (kingPos != NoSquare) ? (controlledByEnemy & (1ull << kingPos)): false;

}

bool SpellChessLib::MoveGenerator::doesMoveSatisfyJumpConstraint(const SpellChessMove& move, const SquareIndex& requiredJumpSq)
{
	return move.jumpSquare == requiredJumpSq;
}

bool SpellChessLib::MoveGenerator::doesMoveSatisfyCheckConstraint(const SpellChessMove& move, const uint64_t checkingPieces)
{
	uint64_t dealtWithPieces = _Constraints::getBlockedAttacker(move.to, kingPos, checkingPieces)
		| _Constraints::getCapturedAttacker(move.to, checkingPieces)
		| _Constraints::getFrozenAttackers(move.freezeSquare, checkingPieces);
	return dealtWithPieces == checkingPieces;
}

bool SpellChessLib::MoveGenerator::doesMoveSatisfyPinConstraint(const SpellChessMove& move)
{
	SquareIndex pinningPieceIndex = pins[move.to];

	if (pinningPieceIndex == NoSquare)
		return true;

	return !_Constraints::isMoveViolatingPin(move.to, kingPos, pinningPieceIndex) || _Constraints::doFreezeFrozeAllAttackers(move.to, 1ull<<pinningPieceIndex);
}

void SpellChessLib::MoveGenerator::generateProtoMovesForSliderRay(const GameState& state, const int & x, const int & y, std::vector<ProtoMove>& out, std::vector<Constrain>& constrains, const int& dx, const int& dy) const
{
	bool jmp_constrains_added = false;
	int i = 1;
	SquareIndex currentSquareIndex;
	SquareIndex sq = squareIndexFromCords(x, y);
	auto us = state.getSideToMove();
	while (CORDINATE_BOUNDARY_CHECK(x + i * dx) && CORDINATE_BOUNDARY_CHECK(y + i * dy)) {
		currentSquareIndex = squareIndexFromCords(x + i * dx, y + i * dy);

		const PlayerColor& pieceOwner = state.getSquare(currentSquareIndex).piece.owner;
		if ( pieceOwner == Nobody) {
			out.emplace_back(sq, currentSquareIndex, constrains);
			goto NEXT_ITER;
		}

		if(pieceOwner != us)
			out.emplace_back(sq, currentSquareIndex, constrains);

		if (!state.getSquare(currentSquareIndex).jumpable) {
			if (jmp_constrains_added)
				break;
			jmp_constrains_added = true;
			constrains.push_back({ CastJumpSq, currentSquareIndex });
		}

		NEXT_ITER:
		++i;
	}

	if (jmp_constrains_added) constrains.pop_back();
}

#define INIT_CONSTRAINS() std::vector<Constrain> constrains; \
	if (inCheck)  constrains.push_back({ NeutralizeCheck, kingPos }); \
	if (pins[sq] != NoSquare) constrains.push_back({ NeutralizePin, pins[sq] });

void SpellChessLib::MoveGenerator::generateProtoMovesForPawn(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove> & out) const
{
	INIT_CONSTRAINS();
	std::vector<Constrain> jumpConstrains;
	auto us = state.getSideToMove();
	int x = sq % 8, y = sq / 8;
	int dy = us == White ? -1 : 1;
	int start_y = us == White ? 6 : 1;
	SquareIndex currentSquare = squareIndexFromCords(x, y + dy);

	if (state.getSquare(currentSquare).isEmpty()) {
		out.emplace_back(sq, currentSquare, constrains);
	}
	
	if (y == start_y) {
		jumpConstrains = constrains;
		if (!state.getSquare(currentSquare).isEmpty() && !state.getSquare(currentSquare).jumpable)
			jumpConstrains.push_back({CastJumpSq, currentSquare});
		currentSquare = squareIndexFromCords(x, y + 2*dy);
		if(state.getSquare(currentSquare).isEmpty())
			out.emplace_back(sq, currentSquare, jumpConstrains);
	}

	currentSquare = squareIndexFromCords(x - 1, y + dy);
	if (CORDINATE_BOUNDARY_CHECK(x - 1) && state.getSquare(currentSquare).piece.owner == flipColor(us) ) {
		out.emplace_back(sq, currentSquare, constrains);
	}

	if (CORDINATE_BOUNDARY_CHECK(x - 1) && currentSquare == state.getEnppassantSquare()) {
		constrains.push_back({ NeutralizeDiscoveredCheckByEnpassant, NoSquare });
		out.emplace_back(sq, currentSquare, constrains);
		constrains.pop_back();
	}

	currentSquare = squareIndexFromCords(x + 1, y + dy);
	if (CORDINATE_BOUNDARY_CHECK(x + 1) && state.getSquare(currentSquare).piece.owner == flipColor(us)) {
		out.emplace_back(sq, currentSquare, constrains);
	}

	if (CORDINATE_BOUNDARY_CHECK(x + 1) && currentSquare == state.getEnppassantSquare()) {
		constrains.push_back({ NeutralizeDiscoveredCheckByEnpassant, NoSquare });
		out.emplace_back(sq, currentSquare, constrains);
		constrains.pop_back();
	}

}

void SpellChessLib::MoveGenerator::generateProtoMovesForKnight(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const
{
#define KNIGHT_MOVE_CASE(dx, dy) currentSquare = squareIndexFromCords(x + dx, y + dy); \
		if (CORDINATE_BOUNDARY_CHECK(x + dx) && CORDINATE_BOUNDARY_CHECK(y + dy) && state.getSquare(currentSquare).piece.owner != us) \
			out.emplace_back(sq, currentSquare, constrains);

	INIT_CONSTRAINS();
	auto us = state.getSideToMove();
	int x = sq % 8, y = sq / 8;
	SquareIndex currentSquare;
	
	KNIGHT_MOVE_CASE(-2, -1);
	KNIGHT_MOVE_CASE(-2, 1);
	KNIGHT_MOVE_CASE(2, -1);
	KNIGHT_MOVE_CASE(2, 1);
	KNIGHT_MOVE_CASE(-1, -2);
	KNIGHT_MOVE_CASE(-1, 2);
	KNIGHT_MOVE_CASE(1, -2);
	KNIGHT_MOVE_CASE(1, 2);

#undef KNIGHT_MOVE_CASE
}

void SpellChessLib::MoveGenerator::generateProtoMovesForKing(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const
{
#define KING_MOVE_CASE(dx, dy) currentSquare = squareIndexFromCords(x + dx, y + dy); \
	if (CORDINATE_BOUNDARY_CHECK(x + dx) && CORDINATE_BOUNDARY_CHECK(y + dy) && state.getSquare(currentSquare).piece.owner != us) { \
	constrains.push_back({NeutralizeSqAttackers, currentSquare});	\
	constrains.push_back({HasNoKingAttackers, currentSquare}); \
	out.emplace_back(sq, currentSquare, constrains); \
	} \
	constrains.clear();

	std::vector<Constrain> constrains;
	auto us = state.getSideToMove();
	auto them = flipColor(us);
	int x = sq % 8, y = sq / 8;
	SquareIndex currentSquare;

	KING_MOVE_CASE(-1, -1);
	KING_MOVE_CASE(-1, 0);
	KING_MOVE_CASE(-1, 1);
	KING_MOVE_CASE(0, -1);
	KING_MOVE_CASE(0, 1);
	KING_MOVE_CASE(1, -1);
	KING_MOVE_CASE(1, 0);
	KING_MOVE_CASE(1, 1);
#undef KING_MOVE_CASE
	
	// kingside castling
	if (currentState->canCastle(us, Kingside)
		&& currentState->getSquare(kingPos + 1).isEmpty()
		&& currentState->getSquare(kingPos + 2).isEmpty()) {
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos) });
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos + 1) });
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos + 2) });
		out.emplace_back(kingPos, kingPos + 2, constrains);
		constrains.clear();
	}

	// queenside castling
	if (currentState->canCastle(us, QueenSide)
		&& currentState->getSquare(kingPos - 1).isEmpty()
		&& currentState->getSquare(kingPos - 2).isEmpty()
		&& currentState->getSquare(kingPos - 3).isEmpty()) {
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos) });
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos - 1) });
		constrains.push_back({ NeutralizeSqAttackers, SquareIndex(kingPos - 2) });
		out.emplace_back(kingPos, kingPos - 2, constrains);
		constrains.clear();
	}

}

void SpellChessLib::MoveGenerator::generateProtoMovesForRook(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const
{
	int x = sq % 8, y = sq / 8;
	INIT_CONSTRAINS();
	generateProtoMovesForSliderRay(state, x, y, out, constrains, -1,  0);
	generateProtoMovesForSliderRay(state, x, y, out, constrains,  1,  0);
	generateProtoMovesForSliderRay(state, x, y, out, constrains,  0, -1);
	generateProtoMovesForSliderRay(state, x, y, out, constrains,  0,  1);
}

void SpellChessLib::MoveGenerator::generateProtoMovesForBishop(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const
{
	int x = sq % 8, y = sq / 8;
	INIT_CONSTRAINS();
	generateProtoMovesForSliderRay(state, x, y, out, constrains, -1, -1);
	generateProtoMovesForSliderRay(state, x, y, out, constrains, -1, 1);
	generateProtoMovesForSliderRay(state, x, y, out, constrains, 1, -1);
	generateProtoMovesForSliderRay(state, x, y, out, constrains, 1, 1);
}

void SpellChessLib::MoveGenerator::generateProtoMoves(const GameState& state, std::vector<ProtoMove>& out) const
{
	auto us = state.getSideToMove();
	for (SquareIndex sq = 0; sq < 64; ++sq) 
		if(state.getSquare(sq).piece.owner == us && !state.getSquare(sq).frozen)
			switch (state.getSquare(sq).piece.type)
			{
				case Pawn:
					generateProtoMovesForPawn(state, sq, out); 
					break;
				case Knight:
					generateProtoMovesForKnight(state, sq, out);
					break;
				case Bishop:
					generateProtoMovesForBishop(state, sq, out);
					break;
				case Rook:
					generateProtoMovesForRook(state, sq, out);
					break;
				case Queen:
					generateProtoMovesForBishop(state, sq, out);
					generateProtoMovesForRook(state, sq, out);
					break;
				case King:
					generateProtoMovesForKing(state, sq, out);
					break;
			default:
				break;
			}
}

void SpellChessLib::MoveGenerator::expandProtoMove(const ProtoMove& protoMove, std::vector<MoveCandidate>& out) const
{
	uint64_t attackersToBeNeutralizedByFreeze = 0ull;
	uint64_t tmp;
	bool usesJump = false;
	
	SpellPolicy policy = UserCanAddSpell;
	SpellChessMove base = { protoMove.from, protoMove.to, NoSquare, NoSquare, None };
	SquareIndex sqToNeutralize = kingPos;
	auto& pieceType = currentState->getSquare(base.from).piece.type;
	bool capturesKing = currentState->getSquare(base.to).piece.type == King;

	for (auto it = protoMove.cbegin(); it != protoMove.cend(); ++it) {
		sqToNeutralize = kingPos;
		switch (it->type){
		case CastJumpSq:
			base.jumpSquare = it->affectedSquare;

			// Edge case: Do not case jump on pinned pieces
			if (pins[base.jumpSquare] != NoSquare)
				return;

			usesJump = true;
			policy = SpellGeneratorEnforced;
			break;
		case NeutralizePin:
			if (_Constraints::isMoveViolatingPin(base.to, kingPos, pins[base.from]))
				attackersToBeNeutralizedByFreeze |= 1ull << pins[base.from];
			break;
		case NeutralizeDiscoveredCheckByEnpassant:
			attackersToBeNeutralizedByFreeze |= _Constraints::getDiscoveredAttackerByEnpassant(*currentState, kingPos, base.from);
			break;
		case NeutralizeSqAttackers:
			sqToNeutralize = it->affectedSquare;
			// fall through
		case NeutralizeCheck:
			attackersToBeNeutralizedByFreeze |= getAttackers(*currentState, sqToNeutralize, flipColor(currentState->getSideToMove()));
			attackersToBeNeutralizedByFreeze &= ~_Constraints::getCapturedAttacker(base.to, attackersToBeNeutralizedByFreeze);
			
			// Edge case: enpassant capturing king attacker
			if (pieceType == Pawn && base.to == currentState->getEnppassantSquare())
				 attackersToBeNeutralizedByFreeze &= 
				 ~_Constraints::getCapturedAttacker(calculateEnpassantPawnIndex(base.from, base.to), attackersToBeNeutralizedByFreeze);

			if(pieceType != King)
				attackersToBeNeutralizedByFreeze &= ~_Constraints::getBlockedAttacker(base.to, kingPos, attackersToBeNeutralizedByFreeze);
		
			break;
		case HasNoKingAttackers:
			sqToNeutralize = it->affectedSquare;
			tmp = _Attackers::getKingAttackers(*currentState, sqToNeutralize, flipColor(currentState->getSideToMove()));
			if (tmp != 0)
				return;
		default:
			break;
		}
	}

	// using freeze is irrevelant if we can simply capture enemy king
	if (capturesKing)
		attackersToBeNeutralizedByFreeze = 0;

	// using both spell in single move is illegal
	if (attackersToBeNeutralizedByFreeze != 0 && usesJump)
		return; 

	// moves needs jump to be performed
	if (usesJump && !currentState->canUseSpell(currentState->getSideToMove(), JumpSpell))
		return;

	std::vector<SquareIndex> freezeSquares;

	if (attackersToBeNeutralizedByFreeze != 0) {
		if (!currentState->canUseSpell(currentState->getSideToMove(), FreezeSpell))
			return;

		policy = SpellGeneratorEnforced;
		for (SquareIndex i = 0; i < 64; ++i)
			if (_Constraints::doFreezeFrozeAllAttackers(i, attackersToBeNeutralizedByFreeze) && !_Constraints::doesFreezeContainSquare(i, base.from) && currentState->getFreezeSquare() != i)
				freezeSquares.push_back(i);
	}
	else
		freezeSquares.push_back(NoSquare);
	
	
	static const std::vector<PieceType> promotionTargets = {Knight, Bishop, Rook, Queen};

	for (const auto& sq : freezeSquares) {
		base.freezeSquare = sq;
		
		if (pieceType == Pawn && (base.to < 8 || base.to >= 56)) {
			for (auto& target : promotionTargets) {
				base.promoteTo = target;
				out.push_back({policy, base});
			}
		}
		else
			out.push_back({ policy, base });
	}

}

SpellChessLib::MoveGenerator::MoveGenerator(const GameState& state)
{
	currentState = &state;
	resetCache();
	calculateCache(state);
}

SpellChessLib::SquareIndex SpellChessLib::MoveGenerator::getPinningPiece(const SquareIndex& sq) const
{
	return pins[sq];
}

uint64_t SpellChessLib::MoveGenerator::getEnemyControlMask() const
{
	return controlledByEnemy;
}

std::vector<SpellChessLib::MoveCandidate> SpellChessLib::MoveGenerator::generateMoves() const
{
	std::vector<ProtoMove> proto_moves;
	std::vector<MoveCandidate> result;
	generateProtoMoves(*currentState, proto_moves);
	
	for (const auto& proto_move : proto_moves)
		expandProtoMove(proto_move, result);

	return result;
}

bool SpellChessLib::MoveGenerator::isInCheck() const
{
	return inCheck;
}

bool SpellChessLib::MoveGenerator::isMoveLegal(const SpellChessMove& move) const
{
	auto validMoves = generateMoves();
	
	for (auto validMove : validMoves) {
		if (validMove.move.from == move.from &&
			validMove.move.to == move.to &&
			validMove.move.promoteTo == move.promoteTo) {

			if (validMove.spellPolicy == SpellGeneratorEnforced &&
				validMove.move.freezeSquare == move.freezeSquare &&
				validMove.move.jumpSquare == move.jumpSquare)
				return true;



			bool usesJump = move.jumpSquare != NoSquare;
			bool usesFreeze = move.freezeSquare != NoSquare;

			if (validMove.spellPolicy != UserCanAddSpell) continue;
			if (usesJump && usesFreeze) continue;
			if (usesJump && ( move.jumpSquare >= 64 || !currentState->canUseSpell(currentState->getSideToMove(), JumpSpell))) continue;
			if (usesFreeze && (move.freezeSquare >= 64 || !currentState->canUseSpell(currentState->getSideToMove(), FreezeSpell))) continue;
			if (usesFreeze && _Constraints::doesFreezeContainSquare(move.freezeSquare, move.from)) continue;

			return true;
		}
	}

	return false;
}

bool SpellChessLib::_Constraints::isMoveViolatingPin(const SquareIndex& to, const SquareIndex& kingPos, const SquareIndex& pinningPiece)
{
	// precondition: kingPos and pinningPiece are colinear
	int x = to % 8, y = to / 8;
	int x0 = kingPos % 8, y0 = kingPos / 8;
	int x1 = pinningPiece % 8, y1 = pinningPiece / 8;
	int dx = _sliderStep(x0, x1), dy = _sliderStep(y0, y1);
	int i = 0;
	
	do {
		++i;
		if (x0 + i * dx == x && y0 + i * dy == y)
			return false;
	} while (x0 + i * dx != x1 || y0 + i * dy != y1);

	return true;
}

uint64_t SpellChessLib::_Constraints::getBlockedAttacker(const SquareIndex& to, const SquareIndex& kingPos, const uint64_t& attackers)
{

	int x = to % 8, y = to / 8;
	int x0 = kingPos % 8, y0 = kingPos / 8;

	int dx = _sliderStep(x0, x), dy = _sliderStep(y0, y);
	int i = 1;
	bool reached = false;
	SquareIndex currSq;
	uint64_t mask;

	while (CORDINATE_BOUNDARY_CHECK(x0 + i*dx) && CORDINATE_BOUNDARY_CHECK(y0 + i * dy)) {
		currSq = squareIndexFromCords(x0 + i * dx, y0 + i * dy);
		mask = 1ull << currSq;
		
		if (mask & attackers)
			return reached ? mask : 0;

		if (x0 + i * dx == x && y0 + i * dy == y)
			reached = true;

		++i;
	}

	return 0;
}

uint64_t SpellChessLib::_Constraints::getCapturedAttacker(const SquareIndex& to, const uint64_t& attackers)
{
	return attackers & (1ull<<to);
}

uint64_t SpellChessLib::_Constraints::getFrozenAttackers(const SquareIndex& freezeSq, const uint64_t& attackers)
{
	if(freezeSq == NoSquare)
		return 0;
	
	int x = freezeSq % 8, y = freezeSq / 8;
	uint64_t result = 0;
	for(int dx = -1; dx <= 1; ++dx)
		for (int dy = -1; dy <= 1; ++dy)
			if (CORDINATE_BOUNDARY_CHECK(x + dx) && CORDINATE_BOUNDARY_CHECK(y + dy)) 
				result |= 1ull << squareIndexFromCords(x + dx, y + dy);

	return result & attackers;
}

uint64_t SpellChessLib::_Constraints::getDiscoveredAttackerByEnpassant(const GameState& state, const SquareIndex& movingSideKing, const SquareIndex& capturingPawn)
{
	// precondition enpSq is not NoSquare
	const SquareIndex& enpSq = state.getEnppassantSquare() + (state.getSideToMove() == White ? 8 : -8);
	int x = movingSideKing % 8, y = movingSideKing / 8;
	int x_enp = enpSq % 8, y_enp = enpSq / 8;
	
	if (!arePointsColinear(x, y, x_enp, y_enp))
		return 0;

	int dx = _sliderStep(x, x_enp), dy = _sliderStep(y, y_enp);
	int i = 1;
	
	SquareIndex currSq;

	while (CORDINATE_BOUNDARY_CHECK(x + i * dx) && CORDINATE_BOUNDARY_CHECK(y + i * dy)) {
		currSq = squareIndexFromCords(x + i * dx, y + i * dy);
		
		// threat enpassantSq and capturing square as empty
		if (state.getSquare(currSq).isEmpty() || currSq == capturingPawn || currSq == enpSq) {
			++i;
			continue;
		}
		
		const auto & p = state.getSquare(currSq).piece;

		if (p.owner == state.getSideToMove()) return 0;

		if (p.type == Queen || (dx && dy && p.type == Bishop) || (dx == 0 && p.type == Rook) || (dy == 0 && p.type == Rook))
			return 1ull << currSq;
		return 0;
	}

	return 0;
}


bool SpellChessLib::_Constraints::doFreezeFrozeAllAttackers(const SquareIndex& freeze, const uint64_t& attackers)
{
	return getFrozenAttackers(freeze, attackers) == attackers;
}

bool SpellChessLib::_Constraints::doesFreezeContainSquare(const SquareIndex& freeze, const SquareIndex& sq)
{
	int x1 = freeze % 8, y1 = freeze / 8;
	int x2 = sq % 8, y2 = sq / 8;

	return std::abs(x1 - x2) <= 1 && std::abs(y1 - y2) <= 1;
}
