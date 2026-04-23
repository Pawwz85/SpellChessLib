#include <Form.hpp>
#include <QMessageBox>

void Form::showError()
{
    QMessageBox message(parentWidget());
    message.setText("Invalid input");
    message.setDetailedText(errorMsg);
    message.exec();
}

void Form::raiseError(const QString &errorMsg){
    errorFlag = true;
    this->errorMsg = errorMsg;
}

Form::Form(QWidget * parent) : errorFlag(false), errorMsg(), QWidget(parent)
{}

void Form::submit(){
    post();
    if(errorFlag) {
        showError();
    } 
}

bool Form::resetErrorFlag(){
    bool res = errorFlag;
    errorFlag = false;
    return res;
}


#include <moc_Form.cpp>
