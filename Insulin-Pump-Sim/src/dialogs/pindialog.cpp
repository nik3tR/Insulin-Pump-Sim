#include "pindialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

// Construct dialog based on mode (Set or Verify)
PINDialog::PINDialog(PinMode mode, const QString& expectedPin, QWidget* parent)
    : QDialog(parent), m_expectedPin(expectedPin), m_mode(mode) {
    setWindowTitle(mode == PinMode::Verify ? "Enter PIN" : "Set 4-Digit PIN");

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(mode == PinMode::Verify ? "Enter your PIN:" : "Enter New PIN:", this);
    layout->addWidget(label);

    pinEdit = new QLineEdit(this);
    pinEdit->setEchoMode(QLineEdit::Normal);
    layout->addWidget(pinEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PINDialog::verifyPIN);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PINDialog::reject);
}

// Return entered PIN string
QString PINDialog::enteredPIN() const {
    return pinEdit->text();
}

// Handle PIN validation depending on mode
void PINDialog::verifyPIN() {
    QString pin = pinEdit->text();
    if (m_mode == PinMode::Set) {
        if (pin.length() != 4) {
            QMessageBox::warning(this, "Invalid PIN", "PIN must be 4 digits.");
            return;
        }
        accept();
    } else {
        if (pin == m_expectedPin)
            accept();
        else {
            QMessageBox::warning(this, "Wrong PIN", "The PIN is not correct.");
            pinEdit->clear();
        }
    }
}
