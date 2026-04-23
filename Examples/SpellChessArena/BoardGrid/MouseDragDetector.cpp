#include <MouseDragDetector.hpp>
#include <cmath>

Board::MouseDragDetector::MouseDragDetector() {
    this->currentInteraction.id = 0;
    this->currentInteraction.classification = NotClassified;
    this->currentInteraction.startingLocation = QPointF(0, 0);
};

void Board::MouseDragDetector::buttonPressed(QPointF where) {

    if (this->currentInteraction.id != 0) {
        // There is currently another interaction being proccessed right now,
        // so we need do decide what to do in this specific case. We don't 
        // expect this situation to occur very often but just in case we
        // will simply filter out extraneous inputs
        return; 
    };

    this->currentInteraction.id = this->buttonInteractionSequence++;
    this->currentInteraction.classification = NotClassified;
    this->currentInteraction.startingLocation = where;
};

void Board::MouseDragDetector::buttonReleased(QPointF where) {

    if (this->currentInteraction.id == 0) {
        /*
            I don't know how this code path could arise in any other way than a genuine coding error.
            But just in case we will apply our typical policy: ignore this user input.
        */
        return;
    }

    if (this->currentInteraction.classification == Drag) {
        onDragEnded(where);
    } else {
        onClick(this->currentInteraction.startingLocation);
    };

    this->currentInteraction.id = 0;
    this->currentInteraction.classification = NotClassified;
    this->currentInteraction.startingLocation = QPointF(0, 0);
};

void Board::MouseDragDetector::mouseMoved(QPointF where) {
    if (this->currentInteraction.id == 0) 
        return; 

    float distSquared = std::pow(where.x() - this->currentInteraction.startingLocation.x(), 2) +
                        std::pow(where.y() - this->currentInteraction.startingLocation.y(), 2);

    if (this->currentInteraction.classification == NotClassified &&
        distSquared > dragDistTravelledThreshold * dragDistTravelledThreshold) {
        this->currentInteraction.classification = Drag;
        onDragStarted(this->currentInteraction.startingLocation);
    }

};



#include <moc_MouseDragDetector.cpp>