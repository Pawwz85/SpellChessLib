#include <EvalThermometer.hpp>

#include <QMetaObject>
#include <QPainter>
#include <QPointer>

#include <SpellChess/UCIHelpers.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

double estimateWinProb(const UCILoader::UciScore & score) {
    if (score.getUnit() == UCILoader::UciScore::Mate)
        return score.getValue() >= 0 ? 1.0 : 0.0;

    if (score.getUnit() != UCILoader::UciScore::Centipawn)
        return 0.5;

    const double alpha = -std::log(10.0) / 800.0;
    return 1.0 / (1.0 + std::exp(score.getValue() * alpha));
}

QString formatEvaluation(const UCILoader::UciScore & score) {
    if (score.getUnit() == UCILoader::UciScore::Mate)
        return QStringLiteral("M%1").arg(std::abs(score.getValue()));

    if (score.getUnit() == UCILoader::UciScore::Centipawn)
        return QStringLiteral("%1%2")
            .arg(score.getValue() >= 0 ? QStringLiteral("+") : QString())
            .arg(score.getValue() * 0.01, 0, 'f', 2);

    return QStringLiteral("N/A");
}

QString formatProbability(double probability) {
    return QString::number(probability * 100.0, 'f', 0) + QStringLiteral("%");
}

} // namespace

void EvalThermometer::disconnectEngine() {
    infoBatchHandler->unlinkAll();
    searchRequestHandler->unlinkAll();
}

void EvalThermometer::connectEngine(const EngineServiceEntry & engineEntry) {
    QPointer<EvalThermometer> self(this);
    
    infoBatchHandler = engineEntry.engine->addSearchRequestHandler([self](const UCILoader::SearchRequest<SpellChessLib::SpellChessMove> & request) {
        if (self.isNull())
            return;

        SpellChessLib::GameState nextContext;
        SpellChessLib::FenParser parser;
        try {
            nextContext = parser.fromSpellChessFen(request.fen);
        } catch (const SpellChessLib::FenValidator::InvalidFenException &) {
            try {
                nextContext = parser.fromStandardChessFen(request.fen);
            } catch (const SpellChessLib::FenValidator::InvalidFenException &) {
                nextContext = SpellChessLib::GameState();
            }
        }

        try {
            for (const auto & move : request.moves)
                nextContext.makeMove(move);
        } catch (const SpellChessLib::IllegalMoveException &) {
            return;
        }

        if (!self.isNull()) {
            QMetaObject::invokeMethod(self.data(), [self, nextContext]() {
                if (!self.isNull())
                    self->context = nextContext;
            }, Qt::QueuedConnection);
        }
    });

    searchRequestHandler = engineEntry.engine->addInfoBatchHandler([self](const std::vector<UCILoader::Info<SpellChessLib::SpellChessMove>> & infos) {
        if (self.isNull())
            return;

        for (const auto & info : infos) {
            if (info.getType() != UCILoader::InfoType::Score)
                continue;

            const auto currentScore = info.getAsScore();
            QMetaObject::invokeMethod(self.data(), [self, currentScore]() {
                if (!self.isNull())
                    self->setEvaluation(currentScore, self->context);
            }, Qt::QueuedConnection);
        }
    });
}


EvalThermometer::EvalThermometer(const EngineServiceEntry & engineEntry,
                                 QWidget * parent)
    : QWidget(parent), score(UCILoader::UciScore::fromCentipawns(0)) {
    setMinimumSize(50, 420);
    setAutoFillBackground(false);

    setEngine(engineEntry);
}

void EvalThermometer::setEvaluation(UCILoader::UciScore newScore,
                                    const SpellChessLib::GameState & newContext) {
    updateDisplay(newScore, newContext);
}

void EvalThermometer::updateDisplay(UCILoader::UciScore newScore,
                                    const SpellChessLib::GameState & newContext) {
    score = newScore;
    context = newContext;

    const auto whitePovScore = SpellChessLib::ScorePOV(context.getSideToMove(), score)
                                   .toPov(SpellChessLib::PlayerColor::White)
                                   .getScore();
    whiteProbability = std::clamp(estimateWinProb(whitePovScore), 0.0, 1.0);
    scoreText = formatEvaluation(whitePovScore);
    update();
}

void EvalThermometer::paintEvent(QPaintEvent * event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect bar = rect().adjusted(12, 12, -12, -12);
    const int whiteHeight = qRound(bar.height() * whiteProbability);
    const QRect whiteRegion(bar.left(), bar.top(), bar.width(), whiteHeight);
    const QRect blackRegion(bar.left(), bar.top() + whiteHeight,
                            bar.width(), bar.height() - whiteHeight);

    painter.fillRect(whiteRegion, QColor(QStringLiteral("#f4f4f4")));
    painter.fillRect(blackRegion, QColor(QStringLiteral("#252525")));
    painter.setPen(QPen(QColor(QStringLiteral("#888888")), 1));
    painter.drawRect(bar.adjusted(0, 0, -1, -1));

    painter.setFont(QFont(font().family(), 12, QFont::Bold));

    painter.setPen(whiteProbability >= 0.5 ? Qt::black : Qt::white);
    painter.drawText(bar, Qt::AlignHCenter | Qt::AlignVCenter, scoreText);
}

#include <moc_EvalThermometer.cpp>
