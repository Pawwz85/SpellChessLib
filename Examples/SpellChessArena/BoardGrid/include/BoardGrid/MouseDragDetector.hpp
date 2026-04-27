#pragma once

#include <QObject>
#include <QPointF>
#include "CoordinateSystem.h"

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
            BoardWidgetPoint startingLocation;
        };

        MouseInteractionInfo currentInteraction;

        public:

        MouseDragDetector();

        void buttonPressed(BoardWidgetPoint where);
        void buttonReleased(BoardWidgetPoint where);
        void mouseMoved(BoardWidgetPoint where);

        signals:
        void onClick(BoardWidgetPoint where);
        void onDragStarted(BoardWidgetPoint where);
        void onDragEnded(BoardWidgetPoint where);
    };

};