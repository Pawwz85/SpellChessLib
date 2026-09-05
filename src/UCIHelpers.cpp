#include <SpellChess/UCIHelpers.h>
#include <stdexcept>

namespace SpellChessLib {
    
    ScorePOV::ScorePOV(PlayerColor color, const UCILoader::UciScore & score) : score(score){
        setPOV(color);
    };

    ScorePOV::ScorePOV(const ScorePOV & other): score(other.score), pov(other.pov){};

    void 
    ScorePOV::setPOV(PlayerColor color) {
        if (color == Nobody)
            throw std::logic_error("Score can not assume 'nobody' as POV");
        bool shouldFlip = color != this->pov;
        this->pov = color;

        if (shouldFlip)
            score = UCILoader::UciScore(score.getBoundType(), score.getUnit(), -score.getValue());
    }

    ScorePOV
    ScorePOV::flipPOV() const {
        PlayerColor otherColor = (pov == White) ? Black : White;
        return ScorePOV(
            otherColor,
            UCILoader::UciScore(score.getBoundType(), score.getUnit(), -score.getValue())
        );
    };

    ScorePOV
    ScorePOV::toPov(PlayerColor color) const {
        ScorePOV copy = *this;
        copy.setPOV(color);
        return copy;
    };


    ScorePOV & ScorePOV::operator=(const ScorePOV & other) {
        copy(other);
        return *this;
    }

}