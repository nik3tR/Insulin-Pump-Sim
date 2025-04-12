#include "newprofiledialog.h"

NewProfileDialog::NewProfileDialog(QWidget* parent) : QDialog(parent) {
    initializeUI("Create New Profile");
}

//Get information to edit new profile
NewProfileDialog::NewProfileDialog(const QString& name, float basal, float carb, float correction, float target, QWidget* parent)
    : QDialog(parent)
{
    initializeUI("Edit Profile");
    nameEdit->setText(name);
    basalEdit->setText(QString::number(basal));
    carbEdit->setText(QString::number(carb));
    correctionEdit->setText(QString::number(correction));
    targetEdit->setText(QString::number(target));
}

QString NewProfileDialog::getName() const {
    return nameEdit->text();
}

float NewProfileDialog::getBasalRate() const {
    return basalEdit->text().toFloat();
}

float NewProfileDialog::getCarbRatio() const {
    return carbEdit->text().toFloat();
}

float NewProfileDialog::getCorrectionFactor() const {
    return correctionEdit->text().toFloat();
}

float NewProfileDialog::getTargetGlucose() const {
    return targetEdit->text().toFloat();
}


//Display the personal information
void NewProfileDialog::initializeUI(const QString& title) {
    setWindowTitle(title);
    QFormLayout* formLayout = new QFormLayout(this);
    nameEdit = new QLineEdit(this);
    basalEdit = new QLineEdit(this);
    carbEdit = new QLineEdit(this);
    correctionEdit = new QLineEdit(this);
    targetEdit = new QLineEdit(this);
    formLayout->addRow("Name:", nameEdit);
    formLayout->addRow("Basal Rate (u/hr):", basalEdit);
    formLayout->addRow("Carb Ratio (1u per X g):", carbEdit);
    formLayout->addRow("Correction Factor (1u per X mmol/L):", correctionEdit);
    formLayout->addRow("Target BG (mmol/L):", targetEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewProfileDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewProfileDialog::reject);
}
