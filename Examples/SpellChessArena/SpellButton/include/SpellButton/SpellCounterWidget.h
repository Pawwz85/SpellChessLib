#pragma once
#include "SpellCounterModel.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <QPushButton>

namespace SpellCounter {

    class SpellCounterWidget : public QWidget {
        Q_OBJECT
        QVBoxLayout layout;
        QPushButton chooseSpellButton;
        QPushButton cancelSpellButton;
        
        QImage  icon;

        public:
        SpellCounterModel model;
        SpellCounterWidget(QWidget * parent = nullptr);

        public slots:
        void onModelChanged();
    };

};