#include <SpriteSet.hpp>
#include <QFile>

void SpriteSet::SpriteSet::deleteAllSprites(){
    for (auto & ptr : sprites)
        delete ptr;
    sprites.clear();
};

SpriteSet::SpriteSet::~SpriteSet() {
    deleteAllSprites();
};

void SpriteSet::SpriteSet::setSchema(const SpriteSetScheme & newSchema){
    
    if(!scheme.isEmpty() && !scheme.validateOther(newSchema)) {
        throw BadSchemaError();
        return;
    };

    bool failureFlag = false;
    std::vector<const QPixmap *> newSprites;

    QUrl failledUrl;

    for (const auto & def : newSchema.getDefinitions()) {
        if (const auto sprite = findPixmapByUrl(def.url)) {
            newSprites.push_back(sprite);
        } else {
            failureFlag = true;
            failledUrl = def.url;
            break;
        }
    };

    if(failureFlag) {
        for (const QPixmap * ptr : newSprites)
            delete ptr;
        throw SpriteNotFoundError(failledUrl);
    } else {
        deleteAllSprites();
        scheme.copyFromOther(newSchema);
        sprites = newSprites;
        onSpriteSetReloaded();
    };

};

const QPixmap * SpriteSet::SpriteSet::findPixmapByUrl(const QUrl & url) const {
    if (!url.isValid())
        return nullptr;

    if (url.isLocalFile()) {
        return new QPixmap(url.toLocalFile());
    };
    
    return nullptr;
};

size_t SpriteSet::SpriteSet::findSpriteIndex(const QString & spriteName) const {
    const auto & defs = scheme.getDefinitions();
    
    for(size_t i = 0; i < defs.size(); ++i)
        if (defs[i].spriteIdentifier == spriteName)
            return i;

    return size_t(-1);
};

#include <moc_SpriteSet.cpp>
