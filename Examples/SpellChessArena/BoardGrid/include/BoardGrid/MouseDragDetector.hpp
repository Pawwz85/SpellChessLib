#pragma once

#include <QObject>
#include <QPointF>

namespace Board {

    class MouseDragDetector : public QObject {
        Q_OBJECT
        
        const float dragDistTravelledThreshold = 10.f;
        unsigned int buttonInteractionSequence = 1;

        enum InteractionClassification {
            NotClassified,
            Click,
            Drag
        };

        struct MouseInteractionInfo {
            unsigned int id;
            InteractionClassification classification;
            QPointF startingLocation;
        };

        MouseInteractionInfo currentInteraction;

        public:

        MouseDragDetector();

        void buttonPressed(QPointF where);
        void buttonReleased(QPointF where);
        void mouseMoved(QPointF where);

        signals:
        void onClick(QPointF where);
        void onDragStarted(QPointF where);
        void onDragEnded(QPointF where);
    };

};