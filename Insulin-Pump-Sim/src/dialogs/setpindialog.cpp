#include "setpindialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

// Construct dialog to enter new PIN
SetPinDialog::SetPinDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Set 4-Digit PIN");
    QFormLayout* layout = new QFormLayout(this);
    pinEdit = new QLineEdit(this);
    pinEdit->setEchoMode(QLineEdit::Password);
    layout->addRow("Enter New PIN:", pinEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SetPinDialog::verifyPin);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SetPinDialog::reject);
}

QString SetPinDialog::getPin() const {
    return pinEdit->text();
}

// Validate 4-digit PIN
void SetPinDialog::verifyPin() {
    if(pinEdit->text().length() != 4) {
        QMessageBox::warning(this, "Invalid PIN", "PIN 4 digits only.");
        return;
    }
    accept();
}

