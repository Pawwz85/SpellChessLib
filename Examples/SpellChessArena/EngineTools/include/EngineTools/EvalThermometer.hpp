#pragma once

#include <memory>
#include <mutex>

#include <QWidget>

#include <SpellChess/Board.h>
#include <SpellChess/SpellChessUciLoaderPort.h>

#include "RunningEnginesService.hpp"

class EvalThermometer : public QWidget {
    Q_OBJECT

    SpellChessLib::GameState context;
    UCILoader::UciScore score;
    double whiteProbability = 0.5;
    QString scoreText = QStringLiteral("0.00");

    void updateDisplay(UCILoader::UciScore score,
                       const SpellChessLib::GameState & context);

protected:
    void paintEvent(QPaintEvent * event) override;

public:
    explicit EvalThermometer(std::shared_ptr<ChessEngine> engine,
                             QWidget * parent = nullptr);

public slots:
    void setEvaluation(UCILoader::UciScore score,
                       const SpellChessLib::GameState & context);
};
