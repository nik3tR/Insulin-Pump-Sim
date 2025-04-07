#include "src/dialogs/boluscalculationdialog.h"
#include "src/logic/bolusmanager.h"
#include "extendedbolusdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <iostream>

// Dialog for computing bolus based on carbs/BG, extended bolus, and result preview
BolusCalculationDialog::BolusCalculationDialog(Profile* profile,
                                               IOB* iob,
                                               InsulinCartridge* cartridge,
                                               CGMSensor* sensor,
                                               QWidget* parent)
    : QDialog(parent),
    m_profile(profile),
    m_finalBolus(0.0),
    m_iob(iob),
    m_cartridge(cartridge),
    m_sensor(sensor),
    m_manager(profile, iob)
{
    setWindowTitle("Bolus Calculator");
    stackedWidget = new QStackedWidget(this);

    // Input page
    QWidget* inputPage = new QWidget(this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputPage);
    QLabel* titleLabel = new QLabel("--- BOLUS CALCULATOR ---\nEnter Meal Info:", inputPage);
    inputLayout->addWidget(titleLabel);

    QFormLayout* formLayout = new QFormLayout();
    carbsEdit = new QLineEdit(inputPage);
    bgEdit    = new QLineEdit(inputPage);
    if (m_sensor) bgEdit->setText(QString::number(m_sensor->getGlucoseLevel()));
    formLayout->addRow("Carbohydrates (g):", carbsEdit);
    formLayout->addRow("Current BG (mmol/L):", bgEdit);
    inputLayout->addLayout(formLayout);

    QHBoxLayout* inputButtonLayout = new QHBoxLayout();
    QPushButton* calculateButton = new QPushButton("Calculate Bolus", inputPage);
    QPushButton* cancelButton1  = new QPushButton("Cancel", inputPage);
    inputButtonLayout->addWidget(calculateButton);
    inputButtonLayout->addWidget(cancelButton1);
    inputLayout->addLayout(inputButtonLayout);
    stackedWidget->addWidget(inputPage);

    // Result page
    QWidget* resultPage = new QWidget(this);
    QVBoxLayout* resultLayout = new QVBoxLayout(resultPage);
    resultLabel = new QLabel(resultPage);
    resultLayout->addWidget(resultLabel);
    formulaLabel = new QLabel(resultPage);
    QFont smallFont = formulaLabel->font();
    smallFont.setPointSize(smallFont.pointSize() - 2);
    formulaLabel->setFont(smallFont);
    formulaLabel->setText("Formulas:\nCarb Bolus = Carbs / ICR\nCorrection = (BG - Target) / CF\nFinal = Total - IOB");
    resultLayout->addWidget(formulaLabel);

    QHBoxLayout* resultButtonLayout = new QHBoxLayout();
    manualButton   = new QPushButton("Immediate Bolus", resultPage);
    extendedButton = new QPushButton("Extended Bolus", resultPage);
    QPushButton* cancelButton2 = new QPushButton("Cancel", resultPage);
    resultButtonLayout->addWidget(manualButton);
    resultButtonLayout->addWidget(extendedButton);
    resultButtonLayout->addWidget(cancelButton2);
    resultLayout->addLayout(resultButtonLayout);
    stackedWidget->addWidget(resultPage);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    connect(cancelButton1, &QPushButton::clicked, this, &QDialog::reject);
    connect(cancelButton2, &QPushButton::clicked, this, &QDialog::reject);
    connect(calculateButton, &QPushButton::clicked, this, &BolusCalculationDialog::calculateBolus);

    connect(manualButton, &QPushButton::clicked, this, [this]() {
        std::cout << "[BolusCalculationDialog] Manual Bolus selected.\n";
        emit immediateBolusParameters(m_finalBolus);
        accept();
    });

    connect(extendedButton, &QPushButton::clicked, this, [this]() {
        ExtendedBolusDialog extDlg(this);
        if (extDlg.exec() == QDialog::Accepted) {
            double duration      = extDlg.getDuration();
            double immediatePct  = extDlg.getImmediatePercentage();
            double extendedPct   = extDlg.getExtendedPercentage();

            // Calculate via BolusManager
            auto params = m_manager.calculateExtended(m_finalBolus, immediatePct, extendedPct, duration);
            double immediateDose = params.immediateDose;
            double extendedDose  = params.extendedDose;
            double currentRate   = params.ratePerHour;

            if (m_iob)
                m_iob->updateIOB(m_iob->getIOB() + immediateDose);
            if (m_cartridge)
                m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - immediateDose);

            emit extendedBolusParameters(duration, immediateDose, extendedDose, currentRate);
            accept();
        }
    });
}

// Perform bolus calculation and show result page
void BolusCalculationDialog::calculateBolus() {
    bool ok1, ok2;
    double carbs    = carbsEdit->text().toDouble(&ok1);
    double currentBG = bgEdit->text().toDouble(&ok2);
    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "Input Error", "Enter valid numbers for Carbs and BG.");
        return;
    }

    emit mealInfoEntered(currentBG);

    // Compute via BolusManager
    BolusResult res = m_manager.calculateStandard(carbs, currentBG);
    m_finalBolus    = res.finalBolus;

    QString resultText;
    resultText += "--- BOLUS RESULT ---\n";
    resultText += QString("Carbs: %1g | BG: %2 mmol/L | IOB: %3 u\n\n")
                      .arg(carbs)
                      .arg(currentBG)
                      .arg(res.existingIOB);
    resultText += QString("Carb Bolus: %1 u\nCorrection Bolus: %2 u\nTotal: %3 u\n\nFinal: %4 u")
                      .arg(res.carbBolus,       0, 'f', 1)
                      .arg(res.correctionBolus, 0, 'f', 1)
                      .arg(res.totalBolus,      0, 'f', 1)
                      .arg(res.finalBolus,      0, 'f', 1);

    resultLabel->setText(resultText);
    stackedWidget->setCurrentIndex(1);
}
