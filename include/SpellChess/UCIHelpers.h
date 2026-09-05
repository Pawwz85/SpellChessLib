#pragma once

#include <UCILoader/UCI.h>
#include "Board.h"

namespace SpellChessLib {
    
    class ScorePOV {
        PlayerColor pov;
        UCILoader::UciScore score;
        
        inline void copy(const ScorePOV & other) {
            pov = other.pov;
            score = other.score;
        };

        public:
        ScorePOV(PlayerColor color, const UCILoader::UciScore & score);
        ScorePOV(const ScorePOV & other);

        inline PlayerColor getPOV() const {
            return pov;
        } 

        inline const UCILoader::UciScore & getScore() const {
            return score;
        }

        void setPOV(PlayerColor color);
        ScorePOV flipPOV() const;        
        ScorePOV toPov(PlayerColor color) const;
        ScorePOV & operator=(const ScorePOV & other);
    };

};