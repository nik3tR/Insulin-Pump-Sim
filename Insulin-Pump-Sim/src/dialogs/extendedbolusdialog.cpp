#include "extendedbolusdialog.h"

ExtendedBolusDialog::ExtendedBolusDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Extended Bolus");
    QFormLayout* formLayout = new QFormLayout(this);
    durationEdit = new QLineEdit(this);
    immediateEdit = new QLineEdit(this);
    extendedEdit = new QLineEdit(this);
    //default value for extended bolus
    durationEdit->setText("3");    // 3 hours
    immediateEdit->setText("60");  // 60%
    extendedEdit->setText("40");   // 40%
    formLayout->addRow("Duration:", durationEdit);
    formLayout->addRow("Immediate Dose (%):", immediateEdit);
    formLayout->addRow("Extended Dose (%):", extendedEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExtendedBolusDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExtendedBolusDialog::reject);
}

double ExtendedBolusDialog::getDuration() const {
    return durationEdit->text().toDouble();
}

double ExtendedBolusDialog::getImmediatePercentage() const {
    return immediateEdit->text().toDouble();
}

double ExtendedBolusDialog::getExtendedPercentage() const {
    return extendedEdit->text().toDouble();
}
