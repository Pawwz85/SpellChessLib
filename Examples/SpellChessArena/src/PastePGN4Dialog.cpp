#include <PastePGN4Dialog.hpp>

#include <QVBoxLayout>
#include <QDialogButtonBox>

PastePGN4Dialog::PastePGN4Dialog(QWidget * parent) : QDialog(parent) {
    setWindowTitle(tr("Paste your PGN4"));
    setModal(true);

    QVBoxLayout * layout = new QVBoxLayout(this);
    pgnInput = new QTextEdit(this);
    
    connect(pgnInput, &QTextEdit::textChanged, this, [this](){
        pgn4Text = pgnInput->toPlainText().toStdString();
    });
    
    layout->addWidget(pgnInput);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PastePGN4Dialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PastePGN4Dialog::reject);
    
    layout->addWidget(buttonBox);
}

#include <moc_PastePGN4Dialog.cpp>