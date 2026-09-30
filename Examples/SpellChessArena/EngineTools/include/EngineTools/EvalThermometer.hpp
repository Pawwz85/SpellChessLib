#pragma once

#include <memory>
#include <mutex>

#include <QWidget>

#include <SpellChess/Board.h>
#include <SpellChess/SpellChessUciLoaderPort.h>

#include "RunningEnginesService.hpp"
#include "EngineTool.hpp"

class EvalThermometer : public QWidget, public EngineTools::EngineTool {
    Q_OBJECT

    std::shared_ptr<UCILoader::EventReceiver> infoBatchHandler = nullptr;
    std::shared_ptr<UCILoader::EventReceiver> searchRequestHandler = nullptr;

    SpellChessLib::GameState context;
    UCILoader::UciScore score;
    double whiteProbability = 0.5;
    QString scoreText = QStringLiteral("0.00");

   void disconnectEngine() override;
   void connectEngine(const EngineServiceEntry & engineEntry) override;

   void updateDisplay(UCILoader::UciScore score,
                       const SpellChessLib::GameState & context);

protected:
    void paintEvent(QPaintEvent * event) override;

public:
    explicit EvalThermometer(const EngineServiceEntry & engineEntry,
                             QWidget * parent = nullptr);

public slots:
    void setEvaluation(UCILoader::UciScore score,
                       const SpellChessLib::GameState & context);
};
