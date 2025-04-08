#include "pindialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

// Prompt user to enter existing PIN
PINDialog::PINDialog(const QString& expectedPin, QWidget* parent)
    : QDialog(parent), m_expectedPin(expectedPin) {
    setWindowTitle("Enter PIN");
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Pick a  4-digit PIN:", this);
    layout->addWidget(label);
    pinEdit = new QLineEdit(this);
    pinEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(pinEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PINDialog::verifyPIN);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PINDialog::reject);
}

QString PINDialog::enteredPIN() const {
    return pinEdit->text();
}

// Validate entered PIN
void PINDialog::verifyPIN() {
    if(pinEdit->text() == m_expectedPin)
        accept();
    else {
        QMessageBox::warning(this, "Wrong PIN", "The PIN is not correct.");
        pinEdit->clear();
    }
}
