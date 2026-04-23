#pragma once

#include <SpriteSet/SpriteSet.hpp>
#include <Misc/Singleton.h>

namespace Board {

    extern SpriteSet::SpriteSetScheme makePieceSetSchemeFromFolder(const QString & pieceSetFolder);
    extern SpriteSet::SpriteSetScheme makeDefaultPieceSetScheme();
    
    class PieceSpriteSet : public SpriteSet::SpriteSet, public Misc::Singleton<PieceSpriteSet> {
        public:
        PieceSpriteSet();
    };
};