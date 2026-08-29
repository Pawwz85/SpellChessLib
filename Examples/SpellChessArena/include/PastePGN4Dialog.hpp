#pragma once

#include <QObject>
#include <QDialog>
#include <QTextEdit>

class PastePGN4Dialog : public QDialog {
    Q_OBJECT
    QTextEdit * pgnInput;
    QPushButton * okButton;
    QPushButton * cancelButton;
    std::string pgn4Text;

    public:
    PastePGN4Dialog(QWidget * parent = nullptr);

    inline const std::string & getPGN4() const {return pgn4Text;};
};