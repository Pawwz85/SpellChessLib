#pragma once

#include <QDialog>
#include <QTextEdit>

#include <string>

class PasteFENDialog : public QDialog {
    Q_OBJECT

    QTextEdit *fenInput;
    std::string fenText;

public:
    PasteFENDialog(QWidget *parent = nullptr);

    inline const std::string &getFEN() const { return fenText; }
};