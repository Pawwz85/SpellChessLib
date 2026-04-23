#include <App.hpp>
#include <EngineRepository.hpp>

#include <QtWidgets>
#include <MainWindow.h>
#include <QObject>


int main(int argc, char *argv[]) {
    QApplication qt_app(argc, argv);
    App app(&qt_app);
    app.getWindow()->show();
    return qt_app.exec();
}