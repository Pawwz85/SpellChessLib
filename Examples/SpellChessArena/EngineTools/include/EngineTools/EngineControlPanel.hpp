#pragma once

#include <memory>
#include <QWidget>

#include <SpellChess/SpellChessUciLoaderPort.h>

#include "RunningEnginesService.hpp"

class EngineControlPanel : public QWidget {
    
    QWidget * addCheckBox(UCILoader::EngineOptionProxy & option);
    QWidget * addSpinBox(UCILoader::EngineOptionProxy & proxy);
    QWidget * addComboBox(UCILoader::EngineOptionProxy & proxy);
    QWidget * addStringInput(UCILoader::EngineOptionProxy & proxy);
    QWidget * addButton(UCILoader::EngineOptionProxy & proxy);

    public:
    EngineControlPanel(std::shared_ptr<ChessEngine> engine, QWidget * parent = nullptr);
};