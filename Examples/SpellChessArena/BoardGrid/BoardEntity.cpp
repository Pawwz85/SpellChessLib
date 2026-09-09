#include <BoardEntity.h>

using namespace Board;


std::atomic<unsigned int> Board::BoardEntityIdentifierSequence = 0;

BoardEntity::BoardEntity() : 
    identifier(BoardEntityIdentifierSequence.fetch_add(1)),
    m_size(1.f, 1.f),
    m_location(0.f, 0.f),
    m_draggable(true),
    m_z_level(0),
    sprite(){
};

void BoardEntity::setSprite(const SpriteSet::SpriteReference & newSprite) {
    sprite = newSprite;
    // onSpriteChanged(newSprite);
};

void BoardEntity::setSizeAnimation(std::unique_ptr<QPropertyAnimation> && animation) {

    if(sizeAnimation) {
        sizeAnimation->stop();
    }

    sizeAnimation.reset(animation.release());

    sizeAnimation->setTargetObject(this);
    sizeAnimation->setPropertyName("size");
    sizeAnimation->start();
};

void BoardEntity::setLocationAnimation(std::unique_ptr<QPropertyAnimation> && animation) {

    if(locationAnimation) {
        locationAnimation->stop();
    }

    locationAnimation.reset(animation.release());

    locationAnimation->setTargetObject(this);
    locationAnimation->setPropertyName("location");
    locationAnimation->start();
};

void BoardEntity::setLocation(const LogicalBoardPoint & point) {
    m_location.setX(point.x());
    m_location.setY(point.y());
};

void BoardEntity::setSize(const LogicalBoardSize & size) {
    m_size.setWidth(size.width());
    m_size.setHeight(size.height());
};

void BoardEntity::setRect(const LogicalBoardRect & rect) {
    setLocation(rect.position());
    setSize(rect.size());
}

LogicalBoardPoint BoardEntity::location() const {
    return LogicalBoardPoint(m_location.x(), m_location.y());
}

LogicalBoardSize BoardEntity::size() const {
    return LogicalBoardSize(m_size.width(), m_size.height());
}

 LogicalBoardRect BoardEntity::boundingRect() const {
    return {location(), size()};
}

void BoardEntity::stopSizeAnimation() {
    if( sizeAnimation )
        sizeAnimation->stop();
    sizeAnimation.reset();
};

void BoardEntity::stopLocationAnimation() {
    if( locationAnimation)
        locationAnimation->stop();
    locationAnimation.reset();
}

 const QPixmap * BoardEntity::getSpritePointer() const {
    return sprite.get();
 };

const QPixmap & BoardEntity::getSprite() const {
    return *sprite;
};

unsigned int BoardEntity::getIdentifier() const {
    return identifier;
};

void BoardEntityList::clear() {
    entities.clear();
}

void BoardEntityList::deleteByIdentifier(unsigned int id) {
    entities.remove_if([id](const BoardEntity & e) {
        return e.getIdentifier() == id;
    });
};

BoardEntity * BoardEntityList::findByIdentifier(unsigned int identifier) {
    for (auto & entity : entities) 
        if (entity.getIdentifier() == identifier)
            return &entity;
    return nullptr;
};

const BoardEntity * BoardEntityList::findByIdentifier(unsigned int identifier) const {
    for (auto & entity : entities) 
        if (entity.getIdentifier() == identifier)
            return &entity;
    return nullptr;
};

BoardEntity & BoardEntityList::addNewEntity() {
    return entities.emplace_back();
};


#include <moc_BoardEntity.cpp>

