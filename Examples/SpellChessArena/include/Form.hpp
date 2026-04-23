#pragma once

#include <QString>
#include <optional>
#include <typeindex>
#include <QWidget>


class Form : public QWidget {
    Q_OBJECT
    bool errorFlag;
    QString errorMsg;

    virtual void post() {};
    void showError();

    protected:

    void raiseError(const QString & errorMsg);


    public:
    Form(QWidget * parent);
    virtual ~Form() = default;
    
    void submit();

    bool resetErrorFlag();
 
};
