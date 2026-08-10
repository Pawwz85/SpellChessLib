#pragma once


#include <memory>
#include <QWidget>
#include <QLabel>
#include <QColor>
#include <QLineEdit>

#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/Board.h>

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

class PrincipalVariationPanel : public QWidget {
    Q_OBJECT
    VariationLineWidget * variationLines;
    SpellChessLib::GameState context;

    std::string formatVariationLine(const std::vector<SpellChessLib::SpellChessMove> & moves);

    public:
    PrincipalVariationPanel(std::shared_ptr<ChessEngine> engine, size_t numVariations = 1, QWidget * parent = nullptr);
    ~PrincipalVariationPanel();

    public slots:
    void contextChanged(const SpellChessLib::GameState & context);
};