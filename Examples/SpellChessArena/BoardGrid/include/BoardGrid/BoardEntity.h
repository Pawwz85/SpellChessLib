#pragma once
#include <QObject>
#include <QPixmap>
#include <QSizeF>
#include <QPointF>
#include <list>
#include <chrono>
#include <atomic>
#include <QPropertyAnimation>
#include <SpriteSet/SpriteSet.hpp> // for SpriteReference

namespace Board {

    extern std::atomic<unsigned int> BoardEntityIdentifierSequence;

    class BoardEntity : public QObject {
        Q_OBJECT    
        
        public:

        enum Type {
            Unknown,
            Piece,
            Marker
        };

        private:
        SpriteSet::SpriteReference sprite;

        Q_PROPERTY(QSizeF   size        MEMBER m_size       NOTIFY onSizeChanged);
        Q_PROPERTY(QPointF  location    MEMBER m_location   NOTIFY onLocationChanged);
        Q_PROPERTY(int      z_level     MEMBER m_z_level);
        Q_PROPERTY(bool     draggable   MEMBER m_draggable);

        std::unique_ptr<QPropertyAnimation> sizeAnimation;
        std::unique_ptr<QPropertyAnimation> locationAnimation;
        unsigned int identifier;

        public:
        Type entityType = Unknown;

        BoardEntity();

        void setSprite(const SpriteSet::SpriteReference & newSprite);
        void setSizeAnimation(std::unique_ptr<QPropertyAnimation> && animation);
        void setLocationAnimation(std::unique_ptr<QPropertyAnimation> && animation);
        void stopSizeAnimation();
        void stopLocationAnimation();


        unsigned int getIdentifier() const;
        const QPixmap * getSpritePointer() const;
        const QPixmap & getSprite() const;
        signals:

        void onSizeChanged(QSizeF newSize);
        void onLocationChanged(QPointF newLocation);

        private:
        QSizeF  m_size;
        QPointF m_location;
        int     m_z_level;
        bool    m_draggable;
    };

    #define EXPOSE_MEMBER_METHOD(method) method() {return entities.method();}
    #define EXPOSE_CONST_MEMBER_METHOD(method) method() const { return entities.method();}

    class BoardEntityList  {
        std::list<BoardEntity> entities;

        public:

        using iterator = typename std::list<BoardEntity>::iterator;
        using const_iterator = typename std::list<BoardEntity>::const_iterator;

        BoardEntityList()   {};
        void clear();
        void deleteByIdentifier(unsigned int id);
        BoardEntity * findByIdentifier(unsigned int identifier);
        const BoardEntity * findByIdentifier(unsigned int identifier) const;
        BoardEntity & addNewEntity();
        
        inline iterator EXPOSE_MEMBER_METHOD(begin);
        inline const_iterator EXPOSE_CONST_MEMBER_METHOD(begin);
        inline const_iterator EXPOSE_CONST_MEMBER_METHOD(cbegin);
        inline iterator EXPOSE_MEMBER_METHOD(end);
        inline const_iterator EXPOSE_CONST_MEMBER_METHOD(end);
        inline const_iterator EXPOSE_CONST_MEMBER_METHOD(cend);
    };

    #undef EXPOSE_MEMBER_METHOD
    #undef EXPOSE_CONST_MEMBER_METHOD
};