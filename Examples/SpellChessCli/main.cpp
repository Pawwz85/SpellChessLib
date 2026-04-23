//#include SpellChess/Board.h"
//#include SpellChess/MoveLegality.h"
//#include SpellChess/StringTokenizer.h"
//#include SpellChess/MoveGen.h"
#include "SpellChess/MatchStatistics.h"
#include <iostream>
#include <string>

using namespace SpellChessLib;
using namespace std;


int main() {

	Stats::TrinomialModel model ;

	while (true) {
		cout << " Enter WDL values (space separated integer numbers): ";
		cin >> model.wins >> model.draws >> model.losses;
		cout << "\n";

		cout << "Total games = " << Stats::getNumberOfGames(model) << '\n';
		cout << "Win ratio = " << Stats::winRatio(model) << '\n';
		cout << "Draw ratio = " << Stats::drawRatio(model) << '\n';
		cout << "Elo difference = " << Stats::eloDiff(model) << "\n";
		cout << "LLR (elo0 = 0, elo1 = 2)  = " << Stats::calculateLLR(model, 0, 2) << "\n";
		cout << "elo0 accepted? = " << ((Stats::calculateLLR(model, 0, 2) < Stats::calculateLLRLowerThreshold(0.05, 0.05)) ? "true" : "false") << "\n";
		cout << "elo1 accepted? = " << ((Stats::calculateLLR(model, 0, 2) > Stats::calculateLLRUpperThreshold(0.05, 0.05)) ? "true" : "false") << "\n";
	}

	return 0;
}