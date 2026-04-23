#include <SpriteSetScheme.hpp>
#include <algorithm>

class SchemeComparator {
    public:

    bool operator()(const SpriteSet::SpriteSetScheme::SpriteDef & def1, const SpriteSet::SpriteSetScheme::SpriteDef & def2) const {
        return def1.spriteIdentifier.compare(def2.spriteIdentifier) > 0;
    };
};

SpriteSet::SpriteSetScheme::SpriteSetScheme(const std::vector<SpriteSet::SpriteSetScheme::SpriteDef> & definitions) 
    : defs(definitions) {
    std::sort(defs.begin(), defs.end(), SchemeComparator());
    };

void SpriteSet::SpriteSetScheme::setDefinitions(const std::vector<SpriteDef> & definitions) {
    defs = definitions;
    std::sort(defs.begin(), defs.end(), SchemeComparator());
};

void SpriteSet::SpriteSetScheme::copyFromOther(const SpriteSetScheme & other){
    defs = other.defs;
}

const std::vector<SpriteSet::SpriteSetScheme::SpriteDef> & SpriteSet::SpriteSetScheme::getDefinitions() const{
    return defs;
}


const QUrl * SpriteSet::SpriteSetScheme::findSpriteUrl(const QString & spriteIdentifier) const {
    for(const auto & def : defs) 
        if(def.spriteIdentifier == spriteIdentifier)
            return &def.url;
    return nullptr;
};


