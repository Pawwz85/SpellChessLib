#include <PrincipalVariationsPanel.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <SpellChess/MoveNotation.h>
#include <SpellChess/Board.h>
#include <SpellChess/UCIHelpers.h>
#include <cmath> // for abs

const QString evaluationLabelStylesheet = QString() +
                                   "background-color: %1; color: %2;" +
                                   "border-radius: 8px;" + 
                                   "padding: 6px;" +
                                   "font-size: 14pt;" +
                                   "min-height: 0px;";

const QString evalLineStyleSheet = QString()+
                                 "background-color: #1e1e1e;" +
                                 "border-radius: 8px;" + 
                                 "padding: 6px;" +
                                 "font-size: 14pt;" +
                                 "color: #e0e0e0;" +
                                 "min-height: 0px;";
                                   
QString formatEvaluation(UCILoader::UciScore score) {
    if (score.getUnit() == UCILoader::UciScore::Centipawn) {
        return QString::number(score.getValue() * 0.01f, '.', 2);
    } else if (score.getUnit() == UCILoader::UciScore::Mate) {
        return QString("M%1").arg(std::abs(score.getValue()));
    } else {
        return QString("N/A");
    }
}

QColor pickForegroundColorForEvaluation(QColor backgroundColor) {
   if (backgroundColor.value() > 200) { // Light background -> dark text
        return Qt::GlobalColor::black;
    } else { // Dark background -> light text
        return Qt::GlobalColor::white;
    }

}

QColor pickBackgroundColorForEvaluation(UCILoader::UciScore score, SpellChessLib::GameState context) {

    auto movingSide = context.getSideToMove();

    int value = score.getValue();
    // Engine sends the evaluation from the perspective of the side to move, so we need to invert it if it's black's turn.
    if (movingSide == SpellChessLib::PlayerColor::Black) 
            value = -value;

    if (score.getUnit() == UCILoader::UciScore::Centipawn) {
        if (value > 100) {
            return Qt::GlobalColor::white; 
        } else if (value < -100) {
            return Qt::GlobalColor::black;
        } else {
            return Qt::GlobalColor::gray;
        }
    } else if (score.getUnit() == UCILoader::UciScore::Mate) {
       return (value > 0)? Qt::GlobalColor::white : Qt::GlobalColor::black;
    }

   return Qt::GlobalColor::gray;   
}

VariationLineWidget::VariationLineWidget(QWidget * parent) : QWidget(parent) {
    QHBoxLayout * layout = new QHBoxLayout(this);

    // Set overall widget styling
    setStyleSheet("border: 1px solid white;");

    evaluationLabel = new QLabel(this);
    evaluationLabel->setFixedWidth(50);
    evaluationLabel->setMaximumHeight(50);
    evaluationLabel->setAlignment(Qt::Alignment::enum_type::AlignCenter);

    variationLine = new QLineEdit(this);
    variationLine->setReadOnly(true);
    
    // Style the move notation line with rounded corners and subtle shadow effect  
    variationLine->setStyleSheet(evalLineStyleSheet);
    

    layout->addWidget(evaluationLabel);
    layout->addWidget(variationLine);

    connect(this, &VariationLineWidget::setEval,this, &VariationLineWidget::evalChanged, Qt::QueuedConnection);
    connect(this, &VariationLineWidget::setVariation, this, &VariationLineWidget::variationChanged, Qt::QueuedConnection);

    setEval(UCILoader::UciScore::fromCentipawns(0), SpellChessLib::GameState());
};

void VariationLineWidget::evalChanged(UCILoader::UciScore score, const SpellChessLib::GameState & context) {
    SpellChessLib::ScorePOV whitePOV = SpellChessLib::ScorePOV(context.getSideToMove(), score).toPov(SpellChessLib::PlayerColor::White);
    evaluationLabel->setText(formatEvaluation(whitePOV.getScore()));
    evaluationLabel->setFrameStyle(QFrame::Box);
    QColor evalBackground = pickBackgroundColorForEvaluation(score, context);
    QColor evalForehround = pickForegroundColorForEvaluation(evalBackground);
    
    evaluationLabel->setStyleSheet(evaluationLabelStylesheet.arg(evalBackground.name()).arg(evalForehround.name()));
};

void VariationLineWidget::variationChanged(const std::string & variation) {
    variationLine->setText(QString::fromStdString(variation));
    variationLine->setCursorPosition(0);
}

std::string PrincipalVariationPanel::formatVariationLine(const std::vector<SpellChessLib::SpellChessMove> & moves) {
    std::string variationText;
    SpellChessLib::GameState tempContext = context;

    if (tempContext.getSideToMove() == SpellChessLib::Black)
        variationText += std::to_string(tempContext.getFullMoveCounter()) + "... ";

    for (const SpellChessLib::SpellChessMove & move : moves){
        if (tempContext.getSideToMove() == SpellChessLib::White)
             variationText += std::to_string(tempContext.getFullMoveCounter()) + ". ";
        variationText += SpellChessLib::spellChessMoveToUnicodeString(move, tempContext) + " ";
        tempContext.makeMove(move);
    } 
    return variationText;
}

PrincipalVariationPanel::PrincipalVariationPanel(std::shared_ptr<ChessEngine> engine, size_t numVariations, QWidget * parent) : QWidget(parent), variationLines(nullptr), context(SpellChessLib::GameState()) {
    variationLines = new VariationLineWidget[numVariations];
    QVBoxLayout * layout = new QVBoxLayout(this);

    for (size_t i = 0; i < numVariations; i++){
        variationLines[i].setParent(this);    
        layout->addWidget(variationLines + i);
    }

    QPointer<PrincipalVariationPanel> self(this);
    engine->addInfoBatchHandler([self, numVariations](const std::vector<UCILoader::Info<SpellChessLib::SpellChessMove>> & infos) {
            if (self.isNull())
                return;
            auto *panel = self.data();
            int pvIndex = 0;

            // scan for the MultiPV index in the infos
            for (const auto & info: infos){
                if (info.getType() == UCILoader::InfoType::MultiPv) {
                    auto multiPv = info.getIntegerValue() - 1;
                    if (pvIndex >= 0 && pvIndex < static_cast<int>(numVariations)) 
                        pvIndex = multiPv;
                }
            }

            for (const auto & info: infos) {
                if (info.getType() == UCILoader::InfoType::Pv) {
                    try {
                        std::string variationLine = panel->formatVariationLine(info.getMoveArray());
                        panel->variationLines[pvIndex].setVariation(variationLine);
                    } catch(SpellChessLib::IllegalMoveException e){
                        // If the engine sends an illegal move, we will just ignore it and not update the variation line.
                    };
                    
                    
                } else if (info.getType() == UCILoader::InfoType::Score) {
                        panel->variationLines[pvIndex].setEval(info.getAsScore(), panel->context);
                }
            };
        });

    engine->addSearchRequestHandler([self](const UCILoader::SearchRequest<SpellChessLib::SpellChessMove> & request) {
        if (self.isNull())
            return;
        auto *panel = self.data();
        SpellChessLib::FenParser parser;

        try {
                panel->context = parser.fromSpellChessFen(request.fen);
        }
        catch (const SpellChessLib::FenValidator::InvalidFenException & e) {
            try {
                panel->context = parser.fromStandardChessFen(request.fen);
            }
            catch (const SpellChessLib::FenValidator::InvalidFenException & e) {
                panel->context = SpellChessLib::GameState();
            }
        };

        for (const auto & move : request.moves) {
            panel->context.makeMove(move);
        }

    });
}

PrincipalVariationPanel::~PrincipalVariationPanel(){
    delete [] variationLines;
}

void PrincipalVariationPanel::contextChanged(const SpellChessLib::GameState & context){
    this->context = context;
}

#include <moc_PrincipalVariationsPanel.cpp>