#pragma once
#include <QObject>
#include <QPixmap>
#include "SpriteSetScheme.hpp"
#include <memory> // for shared_ptr 
#include <vector>
#include <stdexcept>

namespace SpriteSet
{
    
    class SpriteReference;

    class SpriteNotFoundError : public std::runtime_error {
        public:
        SpriteNotFoundError(const QUrl & url) : runtime_error("Failed to load sprite from: " + url.toString().toStdString()) {};
    };

    class BadSchemaError : public std::logic_error {
        public:
        BadSchemaError() : logic_error("New sprite set schema doesn't match definitions of the old schema") {};
    };
    
    class EmptySpriteReferenceError : public std::logic_error {
        public:
        EmptySpriteReferenceError() : logic_error("Attempted dereferencing empty sprite reference") {};
    };

    class SpriteSet : public QObject {
        Q_OBJECT
        friend SpriteReference;

        const QPixmap * findPixmapByUrl(const QUrl & url) const;
        void deleteAllSprites();
        size_t findSpriteIndex(const QString & spriteName) const;

        protected:
        SpriteSetScheme scheme;
        std::vector<const QPixmap *> sprites; 

        public:

        SpriteSet() = default;
        SpriteSet(const SpriteSet & other) = delete;
        virtual  ~SpriteSet();
        void operator=(const SpriteSet & other) = delete;
        
        void setSchema(const SpriteSetScheme & newSchema);        
        
        inline SpriteReference getSprite(const QString & spriteName) const;
        inline SpriteReference operator[] (const QString & spriteName) const;


        signals:
        void onSpriteSetReloaded();
    };


    class SpriteReference {
        const SpriteSet * parent;
        size_t index;

        public:
        SpriteReference() : parent(nullptr), index(-1) {};
        SpriteReference(const SpriteSet * parent, size_t index) : parent(parent), index(index) {};
        SpriteReference(const SpriteReference & other) : parent(other.parent), index(other.index) {};
        inline const QPixmap * get() const;
        inline const QPixmap & operator*() const;
        inline SpriteReference & operator=(const SpriteReference & other);
    };

} // namespace SpriteSet


SpriteSet::SpriteReference SpriteSet::SpriteSet::getSprite(const QString & spriteName) const {
    return SpriteReference(this, findSpriteIndex(spriteName));
};

SpriteSet::SpriteReference SpriteSet::SpriteSet::operator[] (const QString & spriteName) const {
    return getSprite(spriteName);
}

SpriteSet::SpriteReference & SpriteSet::SpriteReference::operator=(const SpriteReference & other) {
    this->parent = other.parent;
    this->index = other.index;
    return *this;
};

const QPixmap * SpriteSet::SpriteReference::get() const {
    const QPixmap * result;
    if(parent == nullptr || index > parent->sprites.size())
        return nullptr;
    result = parent->sprites[index];
    return result;
};

const QPixmap & SpriteSet::SpriteReference::operator*() const {
    const QPixmap * result = this->get();
    if(result == nullptr)
        throw EmptySpriteReferenceError();
    return *result;
};
