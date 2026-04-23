#include <QMainWindow>

class MainWindow : public QMainWindow {
    public:
    MainWindow(QWidget * parent = nullptr, Qt::WindowFlags flags = {{{0}}}) : QMainWindow(parent, flags) {  
    };
};