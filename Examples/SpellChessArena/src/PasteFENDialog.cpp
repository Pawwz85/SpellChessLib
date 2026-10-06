#include <PasteFENDialog.hpp>

#include <QDialogButtonBox>
#include <QVBoxLayout>

PasteFENDialog::PasteFENDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Load position from FEN"));
    setModal(true);
    resize(500, 120);

    auto *layout = new QVBoxLayout(this);
    fenInput = new QTextEdit(this);
    fenInput->setPlaceholderText(tr("Enter a standard or Spell Chess FEN"));
    connect(fenInput, &QTextEdit::textChanged, this, [this]() {
        fenText = fenInput->toPlainText().toStdString();
    });
    layout->addWidget(fenInput);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PasteFENDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PasteFENDialog::reject);
    layout->addWidget(buttonBox);
}

#include <moc_PasteFENDialog.cpp>