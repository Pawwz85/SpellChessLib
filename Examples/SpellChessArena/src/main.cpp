#include <App.hpp>
#include <EngineRepository.hpp>

#include <UCILoader/ProcessWrapper.h>

#include <QtWidgets>
#include <MainWindow.h>
#include <QObject>
#include <QFontDatabase>

#include <csignal>

void warmUpTextRendering() {

     // Trigger font database initialization.
    const auto families = QFontDatabase::families();
    Q_UNUSED(families);

    // Sample glyphs application might need to display
    const QString samples =
        QStringLiteral("ASCII ąćęłńóśźż ")
        + QString::fromUtf8("Русский Ελληνικά العربية עברית 中文 日本語 한국어")
        + QString::fromUtf8("\U00002744");

    QFont font = QApplication::font();

    // Trigger fallback selection, shaping, metrics, and glyph lookup.
    QFontMetricsF metrics(font);
    metrics.boundingRect(samples);

    QTextLayout layout(samples, font);
    layout.beginLayout();
    while (layout.createLine().isValid()) {
    }
    layout.endLayout();

    // Force rasterization/upload into a paint device glyph cache.
    QImage image(2048, 256, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setFont(font);
    painter.drawText(image.rect(), Qt::AlignLeft | Qt::AlignVCenter, samples);
}

void tryBootingLiveAnalisis() {
    
};

void signalHandler(int code) {
    UCILoader::ProcessWrapper::killAll();
    exit(code);
}

int main(int argc, char *argv[]) {
    QApplication qt_app(argc, argv);
    std::signal(SIGTERM, signalHandler);
	std::signal(SIGSEGV, signalHandler);
	std::signal(SIGINT, signalHandler);
	std::signal(SIGABRT, signalHandler);
    warmUpTextRendering();

 
    App app(&qt_app);
    app.getWindow()->show();
    app.openDocument(SpellChessLib::createEmptySpellChessDocument());
    return qt_app.exec();
}