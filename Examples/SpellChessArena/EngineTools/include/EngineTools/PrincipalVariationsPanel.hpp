#pragma once


#include <memory>
#include <QWidget>
#include <QLabel>
#include <QColor>
#include <QLineEdit>

#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/Board.h>

#include "EngineTool.hpp"
#include "RunningEnginesService.hpp"

class VariationLineWidget : public QWidget {
    Q_OBJECT
    QLabel * evaluationLabel;
    QLineEdit * variationLine;
    QColor evaluationColor;
    
    private slots:
    void evalChanged(UCILoader::UciScore score, const SpellChessLib::GameState & context);
    void variationChanged(const std::string & variation);

    public:
    VariationLineWidget(QWidget * parent = nullptr);

    signals:
    void setEval(UCILoader::UciScore score, const SpellChessLib::GameState & context);
    void setVariation(const std::string & variation);
};

class PrincipalVariationPanel : public QWidget, public EngineTools::EngineTool {
    Q_OBJECT
    VariationLineWidget * variationLines;
    size_t numVariations;
    SpellChessLib::GameState context;

    std::shared_ptr<UCILoader::EventReceiver> infoBatchHandler = nullptr;
    std::shared_ptr<UCILoader::EventReceiver> searchRequestHandler = nullptr;

    void disconnectEngine() override;
    void connectEngine(const EngineServiceEntry & engineEntry) override;

    std::string formatVariationLine(const std::vector<SpellChessLib::SpellChessMove> & moves);

    public:
    PrincipalVariationPanel(const EngineServiceEntry & engineEntry, size_t numVariations = 1, QWidget * parent = nullptr);
    ~PrincipalVariationPanel();

    public slots:
    void contextChanged(const SpellChessLib::GameState & context);
};