#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QTimer>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTextEdit>
#include <QFont>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <QObject>

//--------------------------------------------------------
// ENUMS
//--------------------------------------------------------

enum PumpState {
    Off,
    On,
    Charging,
    Error
};

class Profile;

//--------------------------------------------------------
// INSULIN PUMP
//--------------------------------------------------------
class InsulinPump {
public:
    PumpState state;
    Profile* currentProfile;
    InsulinPump() : state(Off), currentProfile(nullptr) {}
    void powerOn() {
        state = On;
        std::cout << "[InsulinPump] Pump powered on.\n";
    }
    void powerOff() {
        state = Off;
        std::cout << "[InsulinPump] Pump powered off.\n";
    }
    void navigateTo(const std::string& screen) {
        std::cout << "[InsulinPump] Navigating to " << screen << "\n";
    }
    void updateDisplay() {
        std::cout << "[InsulinPump] Display updated.\n";
    }
};

//--------------------------------------------------------
// BATTERY
//--------------------------------------------------------
class Battery {
public:
    int level;
    Battery() : level(75) {} // For testing -> starting at 75%
    void discharge() { if(level > 0) level -= 10; }
    int getStatus() const { return level; }
    void charge() { if(level < 100) level++; }
};

//--------------------------------------------------------
// INSULINCARTRIDGE
//--------------------------------------------------------
class InsulinCartridge {
public:
    int insulinLevel;
    InsulinCartridge() : insulinLevel(300) {}
    void updateInsulinLevel(int newLevel) { insulinLevel = newLevel; }
    void refill() { insulinLevel = 300; }
    int getInsulinLevel() const { return insulinLevel; }
};

//--------------------------------------------------------
// IOB
//--------------------------------------------------------
class IOB {
public:
    float activeInsulinUnits;
    IOB() : activeInsulinUnits(0.0f) {}
    void updateIOB(float units) { activeInsulinUnits = units; }
    float getIOB() const { return activeInsulinUnits; }
};

//--------------------------------------------------------
// CGMSENSOR
//--------------------------------------------------------
class CGMSensor {
public:
    float currentGlucoseLevel;
    CGMSensor() : currentGlucoseLevel(5.5f) {} // Default
    float getGlucoseLevel() const { return currentGlucoseLevel; }
    void updateGlucoseData(float newLevel) { currentGlucoseLevel = newLevel; }
};

//--------------------------------------------------------
// PROFILE
//--------------------------------------------------------
class Profile {
private:
    std::string name;
    float basalRate;
    float carbRatio;
    float correctionFactor;
    float targetGlucose;
public:
    Profile(const std::string& n, float basal, float carb, float correction, float target)
        : name(n), basalRate(basal), carbRatio(carb),
        correctionFactor(correction), targetGlucose(target) {}
    std::string getName() const { return name; }
    float getBasalRate() const { return basalRate; }
    float getCarbRatio() const { return carbRatio; }
    float getCorrectionFactor() const { return correctionFactor; }
    float getTargetGlucose() const { return targetGlucose; }

    // Allow updating profile settings (for editing)
    void updateSettings(float basal, float carb, float correction, float target) {
        basalRate = basal;
        carbRatio = carb;
        correctionFactor = correction;
        targetGlucose = target;
        std::cout << "[Profile] Updated profile: " << name << "\n";
    }
};

//--------------------------------------------------------
// PROFILE MANAGER
//--------------------------------------------------------
class ProfileManager {
private:
    std::vector<Profile> profiles;
public:
    void createProfile(const Profile& profile) {
        profiles.push_back(profile);
        std::cout << "[ProfileManager] Created profile: " << profile.getName() << "\n";
    }
    const std::vector<Profile>& getProfiles() const { return profiles; }
    Profile* selectProfile(const std::string& name) {
        for (auto& p : profiles) {
            if (p.getName() == name)
                return &p;
        }
        return nullptr;
    }
    void deleteProfile(const std::string& name) {
        auto it = std::remove_if(profiles.begin(), profiles.end(), [&](const Profile& p) {
            return p.getName() == name;
        });
        if(it != profiles.end()){
            std::cout << "[ProfileManager] Deleted profile: " << name << "\n";
            profiles.erase(it, profiles.end());
        }
    }
};

//--------------------------------------------------------
// NEW / EDIT PROFILE DIALOG
//--------------------------------------------------------
class NewProfileDialog : public QDialog {
    Q_OBJECT
public:
    NewProfileDialog(QWidget* parent = nullptr) : QDialog(parent) {
        initializeUI("Create New Profile");
    }
    NewProfileDialog(const QString& name, float basal, float carb, float correction, float target, QWidget* parent = nullptr)
        : QDialog(parent) {
        initializeUI("Edit Profile");
        nameEdit->setText(name);
        basalEdit->setText(QString::number(basal));
        carbEdit->setText(QString::number(carb));
        correctionEdit->setText(QString::number(correction));
        targetEdit->setText(QString::number(target));
    }

    QString getName() const { return nameEdit->text(); }
    float getBasalRate() const { return basalEdit->text().toFloat(); }
    float getCarbRatio() const { return carbEdit->text().toFloat(); }
    float getCorrectionFactor() const { return correctionEdit->text().toFloat(); }
    float getTargetGlucose() const { return targetEdit->text().toFloat(); }

private:
    QLineEdit *nameEdit, *basalEdit, *carbEdit, *correctionEdit, *targetEdit;

    void initializeUI(const QString& title) {
        setWindowTitle(title);
        QFormLayout* formLayout = new QFormLayout(this);

        nameEdit = new QLineEdit(this);
        basalEdit = new QLineEdit(this);
        carbEdit = new QLineEdit(this);
        correctionEdit = new QLineEdit(this);
        targetEdit = new QLineEdit(this);

        // Updated labels per specification
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
};

//--------------------------------------------------------
// EXTENDED BOLUS DIALOG (UI per UML)
//--------------------------------------------------------
class ExtendedBolusDialog : public QDialog {
    Q_OBJECT
public:
    ExtendedBolusDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Extended Bolus");
        QFormLayout* formLayout = new QFormLayout(this);

        durationEdit = new QLineEdit(this);
        immediateEdit = new QLineEdit(this);
        extendedEdit = new QLineEdit(this);

        // Pre-fill with default values per the UML example
        durationEdit->setText("3");    // 3 hours (simulated as 30 seconds)
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
    double getDuration() const { return durationEdit->text().toDouble(); }
    double getImmediatePercentage() const { return immediateEdit->text().toDouble(); }
    double getExtendedPercentage() const { return extendedEdit->text().toDouble(); }
private:
    QLineEdit *durationEdit;
    QLineEdit *immediateEdit;
    QLineEdit *extendedEdit;
};

//--------------------------------------------------------
// EXTENDED BOLUS PROGRESS DIALOG
//--------------------------------------------------------
class ExtendedBolusProgressDialog : public QDialog {
    Q_OBJECT
public:
    // Modified: Now accepts a pointer to IOB to update it in real time.
    ExtendedBolusProgressDialog(double extendedDose, double simulationDuration, double currentRate, IOB* iob, QWidget* parent = nullptr)
        : QDialog(parent), m_extendedDose(extendedDose), m_simulationDuration(simulationDuration), m_currentRate(currentRate), m_iob(iob)
    {
        // Store the initial IOB (which already includes the immediate dose)
        m_initialIOB = m_iob->getIOB();

        setWindowTitle("Extended Bolus Delivery In Progress");
        QVBoxLayout* layout = new QVBoxLayout(this);
        m_label = new QLabel(this);
        m_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_label);
        setLayout(layout);

        m_elapsed = 0;
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ExtendedBolusProgressDialog::updateProgress);
        m_timer->start(1000);  // update every 1 second
        updateProgress();
    }
public slots:
    void updateProgress() {
        m_elapsed += 1; // seconds elapsed
        double remainingTime = m_simulationDuration - m_elapsed;
        if (remainingTime < 0) remainingTime = 0;
        double delivered = m_extendedDose * (m_elapsed / m_simulationDuration);
        double remainingDose = m_extendedDose - delivered;
        if(remainingDose < 0) remainingDose = 0;

        // Update IOB in real time: set IOB to initial IOB + delivered extended dose
        m_iob->updateIOB(m_initialIOB + delivered);

        // Format remainingTime as HH:MM:SS
        int hours = static_cast<int>(remainingTime) / 3600;
        int minutes = (static_cast<int>(remainingTime) % 3600) / 60;
        int seconds = static_cast<int>(remainingTime) % 60;
        QString timeStr = QString("%1:%2:%3")
                              .arg(hours, 2, 10, QLatin1Char('0'))
                              .arg(minutes, 2, 10, QLatin1Char('0'))
                              .arg(seconds, 2, 10, QLatin1Char('0'));

        QString progressText = "EXTENDED BOLUS DELIVERY IN PROGRESS\n"
                               "------------------------------\n"
                               "Remaining Extended Dose: " + QString::number(remainingDose, 'f', 1) + " units\n"
                                                                          "Time Remaining: " + timeStr + "\n"
                                           "Current Rate: ~" + QString::number(m_currentRate, 'f', 1) + " units/hour\n"
                                                                          "------------------------------";
        m_label->setText(progressText);

        if(m_elapsed >= m_simulationDuration) {
            m_timer->stop();
            QMessageBox::information(this, "Delivery Complete", "Extended Bolus delivery complete.");
            close();
        }
    }
private:
    QLabel* m_label;
    QTimer* m_timer;
    double m_extendedDose;
    double m_simulationDuration; // in seconds (simulated duration)
    double m_currentRate;
    int m_elapsed;
    IOB* m_iob;
    double m_initialIOB;
};

//--------------------------------------------------------
// BOLUS CALCULATION DIALOG (Modified to include Meal Info input and Result display)
//--------------------------------------------------------
class BolusCalculationDialog : public QDialog {
    Q_OBJECT
public:
    // Modified: Now accepts an optional Profile pointer and an IOB pointer.
    BolusCalculationDialog(Profile* profile = nullptr, IOB* iob = nullptr, QWidget* parent = nullptr)
        : QDialog(parent), m_profile(profile), m_finalBolus(0.0), m_iob(iob)
    {
        setWindowTitle("Bolus Calculator");
        stackedWidget = new QStackedWidget(this);

        // Page 1: Meal Info Input
        QWidget* inputPage = new QWidget(this);
        QVBoxLayout* inputLayout = new QVBoxLayout(inputPage);
        QLabel* titleLabel = new QLabel("--- BOLUS CALCULATOR ---\nEnter Meal Info:", inputPage);
        inputLayout->addWidget(titleLabel);
        QFormLayout* formLayout = new QFormLayout();
        carbsEdit = new QLineEdit(inputPage);
        bgEdit = new QLineEdit(inputPage);
        formLayout->addRow("Carbohydrates (g):", carbsEdit);
        formLayout->addRow("Current BG (mmol/L):", bgEdit);
        inputLayout->addLayout(formLayout);
        QHBoxLayout* inputButtonLayout = new QHBoxLayout();
        QPushButton* calculateButton = new QPushButton("Calculate Bolus", inputPage);
        QPushButton* cancelButton1 = new QPushButton("Cancel", inputPage);
        inputButtonLayout->addWidget(calculateButton);
        inputButtonLayout->addWidget(cancelButton1);
        inputLayout->addLayout(inputButtonLayout);
        stackedWidget->addWidget(inputPage);

        // Page 2: Bolus Result and Delivery Method Selection
        QWidget* resultPage = new QWidget(this);
        QVBoxLayout* resultLayout = new QVBoxLayout(resultPage);
        resultLabel = new QLabel(resultPage);
        resultLayout->addWidget(resultLabel);

        // Add a small-font label to display the formulas
        formulaLabel = new QLabel(resultPage);
        QFont smallFont = formulaLabel->font();
        smallFont.setPointSize(smallFont.pointSize() - 2);
        formulaLabel->setFont(smallFont);
        formulaLabel->setText("Formulas:\n"
                              "Carb Bolus = Carbs (g) / ICR (g/u)\n"
                              "Correction Bolus = (Current BG - Target BG) / Correction Factor\n"
                              "Total Bolus = Carb Bolus + Correction Bolus\n"
                              "Final Bolus = Total Bolus - IOB");
        resultLayout->addWidget(formulaLabel);

        QHBoxLayout* resultButtonLayout = new QHBoxLayout();
        manualButton = new QPushButton("Manual Bolus", resultPage);
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

        // Connections
        connect(cancelButton1, &QPushButton::clicked, this, &BolusCalculationDialog::reject);
        connect(cancelButton2, &QPushButton::clicked, this, &BolusCalculationDialog::reject);
        connect(calculateButton, &QPushButton::clicked, this, &BolusCalculationDialog::calculateBolus);
        connect(manualButton, &QPushButton::clicked, this, [this]() {
            std::cout << "[BolusCalculationDialog] Manual Bolus selected.\n";
            accept();
        });
        connect(extendedButton, &QPushButton::clicked, this, [this]() {
            std::cout << "[BolusCalculationDialog] Extended Bolus selected.\n";
            ExtendedBolusDialog extDlg(this);
            if(extDlg.exec() == QDialog::Accepted) {
                double duration = extDlg.getDuration();
                double immediatePct = extDlg.getImmediatePercentage();
                double extendedPct = extDlg.getExtendedPercentage();

                double immediateDose = m_finalBolus * immediatePct / 100.0;
                double extendedDose = m_finalBolus * extendedPct / 100.0;
                double currentRate = (duration > 0) ? extendedDose / duration : 0.0;

                // Immediately add Immediate Dose to IOB.
                if(m_iob)
                    m_iob->updateIOB(m_iob->getIOB() + immediateDose);

                // Create ExtendedBolusProgressDialog and pass IOB pointer.
                ExtendedBolusProgressDialog* progressDlg = new ExtendedBolusProgressDialog(extendedDose, duration * 10, currentRate, m_iob, this);
                progressDlg->setAttribute(Qt::WA_DeleteOnClose);
                progressDlg->show();

                // Emit log signal with extended bolus details.
                emit extendedBolusStarted(QString("Extended Bolus Started\nImmediate Dose = %1 u\nExtended Dose = %2 u\nDuration = %3 hrs\nRate Per Hour = %4 u/hr")
                                              .arg(immediateDose, 0, 'f', 1)
                                              .arg(extendedDose, 0, 'f', 1)
                                              .arg(duration, 0, 'f', 1)
                                              .arg(currentRate, 0, 'f', 1));
                accept();
            }
        });
    }

signals:
    void extendedBolusStarted(const QString& message);

private slots:
    void calculateBolus() {
        bool ok1, ok2;
        double carbs = carbsEdit->text().toDouble(&ok1);
        double currentBG = bgEdit->text().toDouble(&ok2);
        if (!ok1 || !ok2) {
            QMessageBox::warning(this, "Input Error", "Please enter valid numbers for Carbohydrates and Current BG.");
            return;
        }
        // Default values per specification.
        double carbRatio = 10.0;
        double correctionFactor = 2.0;
        double targetBG = 6.0;
        if(m_profile) {
            carbRatio = m_profile->getCarbRatio();
            correctionFactor = m_profile->getCorrectionFactor();
            targetBG = m_profile->getTargetGlucose();
        }
        double carbBolus = carbs / carbRatio;
        double correctionBolus = (currentBG > targetBG) ? (currentBG - targetBG) / correctionFactor : 0.0;
        double totalBolus = carbBolus + correctionBolus;
        double iob = 0.0; // assumed
        m_finalBolus = totalBolus - iob;

        QString resultText;
        resultText += "--- BOLUS RESULT ---\n";
        resultText += "Meal Info:\n";
        resultText += "Carbs: " + QString::number(carbs) + "g | Current BG: " + QString::number(currentBG) + " mmol/L | IOB: " + QString::number(iob) + "u\n\n";
        resultText += "Calculation:\n";
        resultText += "Carb Bolus: " + QString::number(carbBolus, 'f', 1) + " u\n";
        resultText += "Correction Bolus: " + QString::number(correctionBolus, 'f', 1) + " u\n";
        resultText += "Total Bolus: " + QString::number(totalBolus, 'f', 1) + " u\n\n";
        resultText += "💉 Final Bolus: " + QString::number(m_finalBolus, 'f', 1) + " u\n";

        resultLabel->setText(resultText);
        stackedWidget->setCurrentIndex(1);
    }

private:
    QStackedWidget* stackedWidget;
    QLineEdit *carbsEdit, *bgEdit;
    QLabel *resultLabel, *formulaLabel;
    QPushButton *manualButton, *extendedButton;
    Profile* m_profile;
    double m_finalBolus;
    IOB* m_iob;
};

//--------------------------------------------------------
// CHARGING DISPLAY DIALOG
//--------------------------------------------------------
class ChargingDisplayDialog : public QDialog {
    Q_OBJECT
public:
    ChargingDisplayDialog(Battery* battery, QWidget* parent = nullptr)
        : QDialog(parent), m_battery(battery)
    {
        setWindowTitle("Charging Interface");
        QVBoxLayout* layout = new QVBoxLayout(this);

        batteryLabel = new QLabel("Battery Level: " + QString::number(m_battery->getStatus()), this);
        batteryLabel->setAlignment(Qt::AlignCenter);

        QPushButton* closeButton = new QPushButton("Close", this);
        layout->addWidget(batteryLabel);
        layout->addWidget(closeButton);
        setLayout(layout);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ChargingDisplayDialog::onTimeout);
        m_timer->start(1000);

        connect(closeButton, &QPushButton::clicked, this, &ChargingDisplayDialog::accept);
    }

public slots:
    void onTimeout() {
        if(m_battery->level < 100) {
            m_battery->charge();
            batteryLabel->setText("Battery Level: " + QString::number(m_battery->getStatus()));
            std::cout << "[ChargingDisplayDialog] Battery level: " << m_battery->getStatus() << "\n";
        } else {
            m_timer->stop();
        }
    }

private:
    Battery* m_battery;
    QLabel* batteryLabel;
    QTimer* m_timer;
};

//--------------------------------------------------------
// HOME SCREEN WIDGET
//--------------------------------------------------------
class HomeScreenWidget : public QWidget {
    Q_OBJECT
public:
    HomeScreenWidget(ProfileManager* profileManager,
                     Battery* battery,
                     InsulinCartridge* cartridge,
                     IOB* iob,
                     CGMSensor* sensor,
                     QWidget* parent = nullptr)
        : QWidget(parent),
        m_profileManager(profileManager),
        m_battery(battery),
        m_cartridge(cartridge),
        m_iob(iob),
        m_sensor(sensor),
        m_currentProfile(nullptr)
    {
        // Pump Status Section
        QHBoxLayout* statusLayout = new QHBoxLayout();

        batteryBox = createStatusBox("Battery", QString::number(m_battery->getStatus()));
        insulinBox = createStatusBox("Insulin", QString::number(m_cartridge->getInsulinLevel()));
        iobBox     = createStatusBox("IOB", QString::number(m_iob->getIOB()));
        cgmBox     = createStatusBox("CGM", QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");

        statusLayout->addWidget(batteryBox);
        statusLayout->addWidget(insulinBox);
        statusLayout->addWidget(iobBox);
        statusLayout->addWidget(cgmBox);

        QGroupBox* statusGroup = new QGroupBox("Pump Status", this);
        statusGroup->setLayout(statusLayout);

        // Personal Profiles Section
        QGroupBox* profileGroup = new QGroupBox("Personal Profiles", this);
        currentProfileLabel = new QLabel("No profile loaded.", this);
        QPushButton* createProfileButton = new QPushButton("Create New Profile", this);
        QPushButton* editProfileButton = new QPushButton("Edit Profile", this);
        QPushButton* deleteProfileButton = new QPushButton("Delete Profile", this);

        QVBoxLayout* profileLayout = new QVBoxLayout();
        profileLayout->addWidget(currentProfileLabel);
        profileLayout->addWidget(createProfileButton);
        profileLayout->addWidget(editProfileButton);
        profileLayout->addWidget(deleteProfileButton);
        profileGroup->setLayout(profileLayout);

        // Event Log Section
        QGroupBox* logGroup = new QGroupBox("Event Log", this);
        m_logTextEdit = new QTextEdit(this);
        m_logTextEdit->setReadOnly(true);
        QVBoxLayout* logLayout = new QVBoxLayout();
        logLayout->addWidget(m_logTextEdit);
        logGroup->setLayout(logLayout);

        // Layout for Profiles and Log side by side
        QHBoxLayout* profileAndLogLayout = new QHBoxLayout();
        profileAndLogLayout->addWidget(profileGroup);
        profileAndLogLayout->addWidget(logGroup);

        connect(createProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onCreateProfile);
        connect(editProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onEditProfile);
        connect(deleteProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onDeleteProfile);

        // Navigation Buttons
        QPushButton* bolusButton = new QPushButton("Bolus", this);
        QPushButton* optionsButton = new QPushButton("Options", this);
        QPushButton* historyButton = new QPushButton("History", this);
        QPushButton* chargeButton = new QPushButton("Charge", this);
        QHBoxLayout* navLayout = new QHBoxLayout();
        navLayout->addWidget(bolusButton);
        navLayout->addWidget(optionsButton);
        navLayout->addWidget(historyButton);
        navLayout->addWidget(chargeButton);

        connect(bolusButton, &QPushButton::clicked, this, &HomeScreenWidget::onBolus);
        connect(optionsButton, &QPushButton::clicked, this, &HomeScreenWidget::onOptions);
        connect(historyButton, &QPushButton::clicked, this, &HomeScreenWidget::onHistory);
        connect(chargeButton, &QPushButton::clicked, this, &HomeScreenWidget::onCharge);

        // Main Layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(statusGroup);
        mainLayout->addLayout(profileAndLogLayout);
        mainLayout->addLayout(navLayout);
        setLayout(mainLayout);
    }

public slots:
    void updateStatus() {
        batteryBox->setText("Battery\n" + QString::number(m_battery->getStatus()));
        insulinBox->setText("Insulin\n" + QString::number(m_cartridge->getInsulinLevel()));
        iobBox->setText("IOB\n" + QString::number(m_iob->getIOB()));
        cgmBox->setText("CGM\n" + QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
    }

    void onCreateProfile() {
        NewProfileDialog dlg(this);
        if(dlg.exec() == QDialog::Accepted) {
            QString name = dlg.getName();
            if(name.trimmed().isEmpty()){
                return;
            }
            float basal = dlg.getBasalRate();
            float carb = dlg.getCarbRatio();
            float correction = dlg.getCorrectionFactor();
            float target = dlg.getTargetGlucose();
            Profile newProfile(name.toStdString(), basal, carb, correction, target);
            m_profileManager->createProfile(newProfile);
            m_currentProfile = m_profileManager->selectProfile(name.toStdString());
            updateProfileDisplay();
        }
    }

    void onEditProfile() {
        if(!m_currentProfile) {
            QMessageBox::warning(this, "Edit Profile", "No profile loaded to edit.");
            return;
        }
        NewProfileDialog dlg(QString::fromStdString(m_currentProfile->getName()),
                             m_currentProfile->getBasalRate(),
                             m_currentProfile->getCarbRatio(),
                             m_currentProfile->getCorrectionFactor(),
                             m_currentProfile->getTargetGlucose(),
                             this);
        if(dlg.exec() == QDialog::Accepted) {
            m_currentProfile->updateSettings(dlg.getBasalRate(),
                                             dlg.getCarbRatio(),
                                             dlg.getCorrectionFactor(),
                                             dlg.getTargetGlucose());
            updateProfileDisplay();
        }
    }

    void onDeleteProfile() {
        if(!m_currentProfile) {
            QMessageBox::warning(this, "Delete Profile", "No profile loaded to delete.");
            return;
        }
        int ret = QMessageBox::question(this, "Delete Profile",
                                        "Are you sure you want to delete the current profile?",
                                        QMessageBox::Yes | QMessageBox::No);
        if(ret == QMessageBox::Yes) {
            m_profileManager->deleteProfile(m_currentProfile->getName());
            m_currentProfile = nullptr;
            updateProfileDisplay();
        }
    }

    void onBolus() {
        // Modified to pass current profile and IOB pointer to BolusCalculationDialog
        BolusCalculationDialog dlg(m_currentProfile, m_iob, this);
        // Connect the extended bolus signal to our log
        connect(&dlg, &BolusCalculationDialog::extendedBolusStarted, this, &HomeScreenWidget::addLog);
        dlg.exec();
    }

    void onOptions() {
        std::cout << "[HomeScreenWidget] Options button clicked.\n";
    }
    void onHistory() {
        std::cout << "[HomeScreenWidget] History button clicked.\n";
    }
    void onCharge() {
        ChargingDisplayDialog dlg(m_battery, this);
        dlg.exec();
        updateStatus();
    }

    void updateProfileDisplay() {
        if(m_currentProfile) {
            currentProfileLabel->setText(
                "Current Profile: \"" + QString::fromStdString(m_currentProfile->getName()) + "\"\n"
                                                                                              "   - Basal Rate: " + QString::number(m_currentProfile->getBasalRate()) + " u/hr\n"
                                                                      "   - Carb Ratio: 1u per " + QString::number(static_cast<int>(m_currentProfile->getCarbRatio())) + " g\n"
                                                                                        "   - Correction Factor: 1u per " + QString::number(static_cast<int>(m_currentProfile->getCorrectionFactor())) + " mmol/L\n"
                                                                                               "   - Target BG: " + QString::number(m_currentProfile->getTargetGlucose()) + " mmol/L"
                );
        } else {
            currentProfileLabel->setText("No profile loaded.");
        }
    }

    // Public slot to add a log entry.
    void addLog(const QString& message) {
        m_logTextEdit->append(message);
    }

private:
    QLabel* createStatusBox(const QString& title, const QString& value) {
        QLabel* box = new QLabel(title + "\n" + value, this);
        box->setFrameStyle(QFrame::Panel | QFrame::Raised);
        box->setAlignment(Qt::AlignCenter);
        box->setFixedSize(120, 80);
        if(title == "Battery")
            box->setStyleSheet("background-color: #ADD8E6; color: black;");
        else if(title == "Insulin")
            box->setStyleSheet("background-color: #87CEFA; color: black;");
        else if(title == "IOB")
            box->setStyleSheet("background-color: #B0E0E6; color: black;");
        else if(title == "CGM")
            box->setStyleSheet("background-color: #AFEEEE; color: black;");
        return box;
    }

    QLabel *batteryBox, *insulinBox, *iobBox, *cgmBox;
    QLabel *currentProfileLabel;
    QTextEdit* m_logTextEdit;
    ProfileManager* m_profileManager;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    Profile* m_currentProfile = nullptr;
};

//--------------------------------------------------------
// MAIN SIMULATOR WIDGET: POWER ON/OFF TOGGLE
//--------------------------------------------------------
class PumpSimulatorMainWidget : public QWidget {
    Q_OBJECT
public:
    PumpSimulatorMainWidget(QWidget* parent = nullptr) : QWidget(parent) {
        m_pump = new InsulinPump();
        m_battery = new Battery();
        m_cartridge = new InsulinCartridge();
        m_iob = new IOB();
        m_sensor = new CGMSensor();
        m_profileManager = new ProfileManager();

        m_powerButton = new QPushButton("Power On", this);
        m_powerButton->setCheckable(true);
        m_powerButton->setStyleSheet("background-color: lightblue; color: black; font-size: 16px; padding: 10px;");

        m_homeScreen = new HomeScreenWidget(m_profileManager, m_battery, m_cartridge, m_iob, m_sensor, this);
        m_homeScreen->setVisible(false);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_powerButton);
        layout->addWidget(m_homeScreen);
        setLayout(layout);

        connect(m_powerButton, &QPushButton::toggled, this, &PumpSimulatorMainWidget::onPowerToggled);
    }

public slots:
    void onPowerToggled(bool checked) {
        if (checked) {
            m_pump->powerOn();
            m_powerButton->setText("Power Off");
            m_homeScreen->setVisible(true);
            m_homeScreen->updateStatus();
        } else {
            m_pump->powerOff();
            m_powerButton->setText("Power On");
            m_homeScreen->setVisible(false);
        }
    }

private:
    InsulinPump* m_pump;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    ProfileManager* m_profileManager;

    QPushButton* m_powerButton;
    HomeScreenWidget* m_homeScreen;
};

//--------------------------------------------------------
// MAIN WINDOW
//--------------------------------------------------------
class MyMainWindow : public QMainWindow {
public:
    MyMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("t:slim X2 Insulin Pump Simulator");
        PumpSimulatorMainWidget* mainWidget = new PumpSimulatorMainWidget(this);
        setCentralWidget(mainWidget);
        resize(650, 550);
    }
};

//--------------------------------------------------------
// MAIN FUNCTION
//--------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyMainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
