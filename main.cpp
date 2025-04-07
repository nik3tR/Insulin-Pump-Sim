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
#include <QDateTime>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <QObject>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>

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
    }
    void powerOff() {
        state = Off;
    }
    void navigateTo(const std::string& screen) {
        std::cout << "Navigating to " << screen << "\n";
    }
    void updateDisplay() {
        std::cout << "Display updated.\n";
    }
};

//--------------------------------------------------------
// BATTERY
//--------------------------------------------------------
class Battery {
public:
    int level;
    Battery() : level(100) {} //fully charged

    //reduce charge by 10 each time
    void discharge() { if(level > 0) level -= 10; if (level < 0) level = 0; }
    int getStatus() const { return level; }

    //charge the battery
    void charge() { if(level < 100) level++; }
};

//--------------------------------------------------------
// INSULINCARTRIDGE
//--------------------------------------------------------
class InsulinCartridge {
public:
    int insulinLevel;
    bool occluded; //occlusion flag


    InsulinCartridge() : insulinLevel(300), occluded(false) {}

    void updateInsulinLevel(int newLevel) { insulinLevel = newLevel; }

    void refill() { insulinLevel = 300; } //Default Insulin

    int getInsulinLevel() const { return insulinLevel; }

    bool isOccluded() const { return occluded; }

    void setOcclusion(bool status) { occluded = status; }
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
    bool connected; //FOR CGM disconnect/connection

    //Connect cgm by default
    CGMSensor() : currentGlucoseLevel(5.5f), connected(true) {}

    float getGlucoseLevel() const { return currentGlucoseLevel; }

    void updateGlucoseData(float newLevel) { currentGlucoseLevel = newLevel; }


    bool isConnected() const { return connected; }
    void disconnectSensor() { connected = false; }
    void connectSensor() { connected = true; }
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

    // Update profile settings (Use Case 3: Personal Profiles - Update)
    void updateSettings(float basal, float carb, float correction, float target) {
        basalRate = basal;
        carbRatio = carb;
        correctionFactor = correction;
        targetGlucose = target;
        std::cout << "Updated profile: " << name << "\n";
    }
};

//--------------------------------------------------------
// PROFILE MANAGER
//--------------------------------------------------------
class ProfileManager {
private:
    std::vector<Profile> profiles;
public:

    //Create profiel
    void createProfile(const Profile& profile) {
        profiles.push_back(profile);
    }

    const std::vector<Profile>& getProfiles() const { return profiles; }
    Profile* selectProfile(const std::string& name) {
        for (auto& p : profiles) {
            if (p.getName() == name)
                return &p;
        }
        return nullptr;
    }

    //Remove Profile
    void deleteProfile(const std::string& name) {
        auto it = std::remove_if(profiles.begin(), profiles.end(), [&](const Profile& p) {
            return p.getName() == name;
        });
        if(it != profiles.end()){
            profiles.erase(it, profiles.end());
        }
    }
};

//--------------------------------------------------------
// MODULAR CLASSES FOR INSULIN DELIVERY
//--------------------------------------------------------

// Adjusts delivery based on CGM reading
class ControlIQ {
public:
    ControlIQ() {}
    double adjustDelivery(double cgm) {

        //if BG is high, full rate else if low, reduce rate.
        return (cgm >= 7.0 ? 1.0 : 0.5);
    }
};


// finds bolus values based on meal info and profile
class BolusManager {
public:
    BolusManager(Profile* profile, IOB* iob) : m_profile(profile), m_iob(iob) {}


    //Use the formulas to find the final bolus for manual bolus calculations (Use Case 4)
    double computeFinalBolus(double carbs, double currentBG) {
        double carbRatio = 10.0, correctionFactor = 2.0, targetBG = 6.0;


        if (m_profile) {
            carbRatio = m_profile->getCarbRatio();
            correctionFactor = m_profile->getCorrectionFactor();
            targetBG = m_profile->getTargetGlucose();
        }



        double carbBolus = carbs / carbRatio;


        double correctionBolus = (currentBG > targetBG) ? (currentBG - targetBG) / correctionFactor : 0.0;
        double totalBolus = carbBolus + correctionBolus;
        double iob = (m_iob) ? m_iob->getIOB() : 0.0;
        double finalBolus = totalBolus - iob;
        if(finalBolus < 0)
            finalBolus = 0;
        return finalBolus;
    }

    // find extended bolus (Use Case 5)
    void computeExtendedBolus(double finalBolus, double immediatePercentage, double &immediateDose, double &extendedDose) {
        immediateDose = finalBolus * immediatePercentage / 100.0;
        extendedDose = finalBolus - immediateDose;
    }

private:
    Profile* m_profile;
    IOB* m_iob;
};

//--------------------------------------------------------
// BasalManager: Use Case 6: start, stop, resume Basal Insulin Delivery
//--------------------------------------------------------
#include <functional>
class BasalManager : public QObject {
    Q_OBJECT
public:
    BasalManager(Profile* profile, Battery* battery, InsulinCartridge* cartridge, IOB* iob, CGMSensor* sensor, QObject* parent = nullptr)
        : QObject(parent), m_profile(profile), m_battery(battery), m_cartridge(cartridge), m_iob(iob), m_sensor(sensor) {}

    // callbacks allow the UI to log events, update status, and change the basal status label.
    void startBasalDelivery(std::function<void(const QString&)> logCallback,
                            std::function<void()> updateStatusCallback,
                            std::function<void(const QString&)> basalStatusCallback) {
        if (!m_profile) {
            logCallback("[BASAL EVENT] No profile loaded.");
            return;
        }
        if (m_battery && m_battery->getStatus() == 0) {
            logCallback("Battery is drained -> Charge the pump.");
            return;
        }
        float rate = m_profile->getBasalRate();
        if (rate <= 0.0f) {
            logCallback("[BASAL EVENT] Set a valid basal rate in the profile to start delivery.");
            return;
        }
        logCallback(QString("[BASAL EVENT] Basal Delivery started at %1 u/hr").arg(rate));

        m_timer = new QTimer(this);
        ControlIQ controlIQ;
        connect(m_timer, &QTimer::timeout, this, [=]() mutable {
            if (m_battery && m_battery->getStatus() == 0) {
                logCallback("Battery fully drained ->  Basal Delivery stopped.");
                basalStatusCallback("Basal Delivery Stopped (Battery 0%)");
                m_timer->stop();
                m_timer->deleteLater();
                return;
            }



            if (m_battery->getStatus() <= 15 && m_battery->getStatus() > 0) {
                logCallback("[SYSTEM EVENT] Low Battery Warning: Battery is low (15%).");
            }



            // Check for CGM disconnection and occlusion alerts
            if(m_sensor && !m_sensor->isConnected()) {
                logCallback("[SYSTEM EVENT] CGM sensor disconnected. Basal delivery suspended.");
                basalStatusCallback("Basal Delivery Suspended (CGM Disconnected)");
                m_timer->stop();
                m_timer->deleteLater();
                return;
            }
            if(m_cartridge && m_cartridge->isOccluded()) {
                logCallback("[SYSTEM EVENT] Occlusion detected. Check infusion site. Basal delivery suspended.");
                basalStatusCallback("Basal Delivery Suspended (Occlusion Detected)");
                m_timer->stop();
                m_timer->deleteLater();
                return;
            }
            float cgm = m_sensor->getGlucoseLevel();
            // Use ControlIQ to adjust the basal rate based on CGM reading
            double adjustment = controlIQ.adjustDelivery(cgm);
            float adjustedRate = rate * adjustment;

            // Pause basal delivery if CGM is too low
            if (cgm < 4.0f) {
                basalStatusCallback("Basal Paused (Low CGM)");
                logCallback("[BASAL EVENT] Basal Delivery Paused — CGM too low (< 3.9 mmol/L)");
                return;
            }
            // Deliver insulin
            if (m_cartridge && m_cartridge->getInsulinLevel() > 0) {
                int insulinLeft = m_cartridge->getInsulinLevel() - adjustedRate;
                m_cartridge->updateInsulinLevel(insulinLeft > 0 ? insulinLeft : 0);
            }
            if (m_iob)
                m_iob->updateIOB(m_iob->getIOB() + adjustedRate);
            if (m_battery)
                m_battery->discharge();
            if (m_sensor) {
                float newCGM = m_sensor->getGlucoseLevel() - 0.1f;
                if (newCGM < 2.5f) newCGM = 2.5f;
                m_sensor->updateGlucoseData(newCGM);
            }
            updateStatusCallback();
            basalStatusCallback(QString("Delivering Basal Insulin @ %1 u/hr").arg(adjustedRate));
            logCallback(QString("[BASAL EVENT] Basal Delivered: %1 u | CGM: %2 mmol/L")
                            .arg(adjustedRate, 0, 'f', 1)
                            .arg(m_sensor->getGlucoseLevel(), 0, 'f', 1));
        });
        m_timer->start(10000);
    }

    void stopBasalDelivery() {
        if (m_timer) {
            m_timer->stop();
            m_timer->deleteLater();
            m_timer = nullptr;
        }
    }

private:
    Profile* m_profile;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    QTimer* m_timer = nullptr;
};

//--------------------------------------------------------
// DATA MANAGER (New for event history logging)
//--------------------------------------------------------
class DataManager {
public:
    void logEvent(const QString& event) {
        QString timeStamped = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + event;
        eventHistory.append(timeStamped);
    }
    QString getHistory() const {
        return eventHistory.join("\n");
    }
    // Placeholder for potential future usage analysis.
    QString analyzeUsage() const {
        return "Usage analysis not implemented.";
    }
private:
    QStringList eventHistory;
};

//--------------------------------------------------------
// SET PIN DIALOG (Simplified version)
//--------------------------------------------------------
class SetPinDialog : public QDialog {
    Q_OBJECT
public:
    SetPinDialog(QWidget* parent = nullptr) : QDialog(parent) {
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
    QString getPin() const { return pinEdit->text(); }
private slots:
    void verifyPin() {
        if(pinEdit->text().length() != 4) {
            QMessageBox::warning(this, "Invalid PIN", "PIN must be 4 digits.");
            return;
        }
        accept();
    }
private:
    QLineEdit* pinEdit;
};

//--------------------------------------------------------
// PIN DIALOG (Modified)
//--------------------------------------------------------
class PINDialog : public QDialog {
    Q_OBJECT
public:
    PINDialog(const QString& expectedPin, QWidget* parent = nullptr) : QDialog(parent), m_expectedPin(expectedPin) {
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
    QString enteredPIN() const { return pinEdit->text(); }
private slots:
    void verifyPIN() {
        if(pinEdit->text() == m_expectedPin)
            accept();
        else {
            QMessageBox::warning(this, "Invalid PIN", "The PIN you entered is incorrect.");
            pinEdit->clear();
        }
    }
private:
    QLineEdit* pinEdit;
    QString m_expectedPin;
};

//--------------------------------------------------------
// NAVIGATION MANAGER (Navigation)
//--------------------------------------------------------
class NavigationManager {
public:
    NavigationManager(QStackedWidget* stack) : m_stack(stack) {}
    void navigateTo(const QString& screen) {
        if (screen == "Home")
            m_stack->setCurrentIndex(0);
        else if (screen == "Options")
            m_stack->setCurrentIndex(1);
        else if (screen == "History")
            m_stack->setCurrentIndex(2);
        else if (screen == "Bolus") {
            std::cout << "[NavigationManager] Bolus view requested.\n";
        }
        std::cout << "[NavigationManager] Navigated to " << screen.toStdString() << "\n";
    }
    void navigateToOptions() { navigateTo("Options"); }
    void navigateToHistory() { navigateTo("History"); }
    void navigateToHome() { navigateTo("Home"); }
    void navigateToBolus() { navigateTo("Bolus"); }
private:
    QStackedWidget* m_stack;
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
// BOLUS CALCULATION DIALOG (with Extended Bolus Simulation)
//--------------------------------------------------------
class BolusCalculationDialog : public QDialog {
    Q_OBJECT
public:
    BolusCalculationDialog(Profile* profile = nullptr, IOB* iob = nullptr, InsulinCartridge* cartridge = nullptr, CGMSensor* sensor = nullptr, QWidget* parent = nullptr)
        : QDialog(parent), m_profile(profile), m_finalBolus(0.0), m_iob(iob), m_cartridge(cartridge), m_sensor(sensor)
    {
        setWindowTitle("Bolus Calculator");
        stackedWidget = new QStackedWidget(this);
        QWidget* inputPage = new QWidget(this);
        QVBoxLayout* inputLayout = new QVBoxLayout(inputPage);
        QLabel* titleLabel = new QLabel("--- BOLUS CALCULATOR ---\nEnter Meal Info:", inputPage);
        inputLayout->addWidget(titleLabel);
        QFormLayout* formLayout = new QFormLayout();
        carbsEdit = new QLineEdit(inputPage);
        bgEdit = new QLineEdit(inputPage);
        if(m_sensor) bgEdit->setText(QString::number(m_sensor->getGlucoseLevel()));
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
        manualButton = new QPushButton("Immediate Bolus", resultPage);
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
            if(extDlg.exec() == QDialog::Accepted) {
                double duration = extDlg.getDuration();
                double immediatePct = extDlg.getImmediatePercentage();
                double immediateDose, extendedDose;
                BolusManager manager(m_profile, m_iob);
                manager.computeExtendedBolus(m_finalBolus, immediatePct, immediateDose, extendedDose);
                double currentRate = (duration > 0) ? extendedDose / duration : 0.0;
                if (m_iob)
                    m_iob->updateIOB(m_iob->getIOB() + immediateDose);
                if (m_cartridge)
                    m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - immediateDose);
                emit extendedBolusParameters(duration, immediateDose, extendedDose, currentRate);
                accept();
            }
        });
    }
    QLineEdit* getCarbsEdit() { return carbsEdit; }
signals:
    void extendedBolusParameters(double duration, double immediateDose, double extendedDose, double ratePerHour);
    void immediateBolusParameters(double immediateDose);
    void mealInfoEntered(double currentBG);
private slots:
    void calculateBolus() {
        bool ok1, ok2;
        double carbs = carbsEdit->text().toDouble(&ok1);
        double currentBG = bgEdit->text().toDouble(&ok2);
        if (!ok1 || !ok2) {
            QMessageBox::warning(this, "Input Error", "Enter valid numbers for Carbs and BG.");
            return;
        }
        emit mealInfoEntered(currentBG);
        BolusManager manager(m_profile, m_iob);
        m_finalBolus = manager.computeFinalBolus(carbs, currentBG);
        QString resultText;
        resultText += "--- BOLUS RESULT ---\n";
        resultText += QString("Carbs: %1g | BG: %2 mmol/L | IOB: %3 u\n\n")
                          .arg(carbs)
                          .arg(currentBG)
                          .arg((m_iob) ? m_iob->getIOB() : 0.0);
        double carbRatio = (m_profile) ? m_profile->getCarbRatio() : 10.0;
        double correctionFactor = (m_profile) ? m_profile->getCorrectionFactor() : 2.0;
        double targetBG = (m_profile) ? m_profile->getTargetGlucose() : 6.0;
        double carbBolus = carbs / carbRatio;
        double correctionBolus = (currentBG > targetBG) ? (currentBG - targetBG) / correctionFactor : 0.0;
        double totalBolus = carbBolus + correctionBolus;
        resultText += QString("Carb Bolus: %1 u\nCorrection Bolus: %2 u\nTotal: %3 u\n\nFinal: %4 u")
                          .arg(carbBolus, 0, 'f', 1)
                          .arg(correctionBolus, 0, 'f', 1)
                          .arg(totalBolus, 0, 'f', 1)
                          .arg(m_finalBolus, 0, 'f', 1);
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
    InsulinCartridge* m_cartridge;
    CGMSensor* m_sensor;
};

//--------------------------------------------------------
// CHARGING DISPLAY DIALOG (unchanged)
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
// HOME SCREEN WIDGET (Modified simulation buttons and History logging)
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
        m_currentProfile(nullptr),
        m_chargingTimer(nullptr)
    {
        // Instantiate DataManager for persistent event logging
        m_dataManager = new DataManager();

        m_mainStackedWidget = new QStackedWidget(this);
        QWidget* homePage = new QWidget(this);
        QVBoxLayout* homeLayout = new QVBoxLayout(homePage);
        QVBoxLayout* statusLayout = new QVBoxLayout();
        m_graph_points = new QScatterSeries();
        m_graph_points->setMarkerSize(11);
        m_graph_points->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        m_graph_points->setColor(Qt::black);
        m_predicted_points = new QScatterSeries();
        m_predicted_points->setMarkerSize(11);
        m_graph_line = new QSplineSeries();
        m_graph_line->setColor(Qt::blue);
        QLineSeries* verticalLine = new QLineSeries();
        verticalLine->append(0, 0);
        verticalLine->append(0, 15);
        verticalLine->setColor(Qt::green);
        m_chart = new QChart();
        m_chart->addSeries(m_graph_points);
        m_chart->addSeries(m_predicted_points);
        m_chart->addSeries(m_graph_line);
        m_chart->addSeries(verticalLine);
        m_chart->createDefaultAxes();
        m_chart->setTitle("CGM readings");
        m_chart->legend()->setVisible(false);
        m_chart->axes()[0]->setRange(-6, 2);
        m_chart->axes()[1]->setRange(2, 11);
        m_chart->axes()[0]->setTitleText("Time (h)");
        m_chart->axes()[1]->setTitleText("Glucose Level (mmol/L)");
        QChartView* chartView = new QChartView(m_chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumSize(QSize(500, 350));
        m_predicted_points->setColor(Qt::gray);
        statusLayout->addWidget(chartView);
        QHBoxLayout* boxDataLayout = new QHBoxLayout();
        batteryBox = createStatusBox("Battery", QString::number(m_battery->getStatus()));
        insulinBox = createStatusBox("Insulin", QString::number(m_cartridge->getInsulinLevel()));
        iobBox     = createStatusBox("IOB", QString::number(m_iob->getIOB()));
        cgmBox     = createStatusBox("CGM", QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
        boxDataLayout->addWidget(batteryBox);
        boxDataLayout->addWidget(insulinBox);
        boxDataLayout->addWidget(iobBox);
        boxDataLayout->addWidget(cgmBox);
        QFrame* boxDataFrame = new QFrame();
        boxDataFrame->setLayout(boxDataLayout);
        statusLayout->addWidget(boxDataFrame);
        QGroupBox* statusGroup = new QGroupBox("Pump Status", homePage);
        statusGroup->setLayout(statusLayout);
        QGroupBox* profileGroup = new QGroupBox("Personal Profiles", homePage);
        currentProfileLabel = new QLabel("No profile loaded.", homePage);
        QPushButton* createProfileButton = new QPushButton("Create New Profile", homePage);
        QPushButton* editProfileButton = new QPushButton("Edit Profile", homePage);
        QPushButton* deleteProfileButton = new QPushButton("Delete Profile", homePage);
        QVBoxLayout* profileLayout = new QVBoxLayout();
        profileLayout->addWidget(currentProfileLabel);
        profileLayout->addWidget(createProfileButton);
        profileLayout->addWidget(editProfileButton);
        profileLayout->addWidget(deleteProfileButton);
        profileGroup->setLayout(profileLayout);
        QGroupBox* logGroup = new QGroupBox("Event Log", homePage);
        m_logTextEdit = new QTextEdit(homePage);
        m_logTextEdit->setReadOnly(true);
        QVBoxLayout* logLayout = new QVBoxLayout();
        logLayout->addWidget(m_logTextEdit);
        logGroup->setLayout(logLayout);
        QHBoxLayout* profileAndLogLayout = new QHBoxLayout();
        profileAndLogLayout->addWidget(profileGroup);
        profileAndLogLayout->addWidget(logGroup);
        QPushButton* bolusButton = new QPushButton("Bolus", homePage);
        QPushButton* optionsButton = new QPushButton("Options", homePage);
        QPushButton* historyButton = new QPushButton("History", homePage);
        QPushButton* chargeButton = new QPushButton("Charge", homePage);
        QPushButton* basalButton = new QPushButton("Start Basal Delivery", homePage);
        // NEW: Simulation buttons now toggle the error condition.
        QPushButton* disconnectButton = new QPushButton("Toggle CGM Disconnect", homePage);
        QPushButton* occlusionButton = new QPushButton("Toggle Occlusion", homePage);
        QHBoxLayout* navLayout = new QHBoxLayout();
        navLayout->addWidget(bolusButton);
        navLayout->addWidget(optionsButton);
        navLayout->addWidget(historyButton);
        navLayout->addWidget(chargeButton);
        navLayout->addWidget(basalButton);
        navLayout->addWidget(disconnectButton);
        navLayout->addWidget(occlusionButton);
        homeLayout->addWidget(statusGroup);
        homeLayout->addLayout(profileAndLogLayout);
        homeLayout->addLayout(navLayout);
        basalStatusLabel = new QLabel("Basal Delivery not started.", homePage);
        basalStatusLabel->setStyleSheet("background-color: lightgreen; padding: 4px;");
        basalStatusLabel->setAlignment(Qt::AlignCenter);
        basalStatusLabel->setFixedHeight(30);
        homeLayout->addWidget(basalStatusLabel);

        // History Screen: replace the simple label with a read-only text edit.
        QWidget* historyPage = new QWidget(this);
        QVBoxLayout* historyLayout = new QVBoxLayout(historyPage);
        m_historyTextEdit = new QTextEdit(historyPage);
        m_historyTextEdit->setReadOnly(true);
        QPushButton* backFromHistory = new QPushButton("Back", historyPage);
        historyLayout->addWidget(m_historyTextEdit);
        historyLayout->addWidget(backFromHistory);

        QWidget* optionsPage = new QWidget(this);
        QVBoxLayout* optionsLayout = new QVBoxLayout(optionsPage);
        QLabel* optionsLabel = new QLabel("Options Screen", optionsPage);
        QPushButton* backFromOptions = new QPushButton("Back", optionsPage);
        optionsLayout->addWidget(optionsLabel);
        optionsLayout->addWidget(backFromOptions);
        m_mainStackedWidget->addWidget(homePage);     // index 0: Home
        m_mainStackedWidget->addWidget(optionsPage);    // index 1: Options
        m_mainStackedWidget->addWidget(historyPage);      // index 2: History
        m_navManager = new NavigationManager(m_mainStackedWidget);
        connect(bolusButton, &QPushButton::clicked, this, &HomeScreenWidget::onBolus);
        connect(optionsButton, &QPushButton::clicked, this, [this]() { m_navManager->navigateToOptions(); });
        // When History is pressed, update the history text from DataManager before navigating.
        connect(historyButton, &QPushButton::clicked, this, [this]() { updateHistory(); m_navManager->navigateToHistory(); });
        connect(chargeButton, &QPushButton::clicked, this, &HomeScreenWidget::onCharge);
        connect(basalButton, &QPushButton::clicked, this, &HomeScreenWidget::startBasalDelivery);
        // NEW: Toggle simulation buttons with system event logging.
        connect(disconnectButton, &QPushButton::clicked, this, [this, disconnectButton]() {
            if(m_sensor->isConnected()){
                m_sensor->disconnectSensor();
                disconnectButton->setText("Toggle CGM Reconnect");
                addLog("[SYSTEM EVENT] Simulated CGM disconnect.");
            } else {
                m_sensor->connectSensor();
                disconnectButton->setText("Toggle CGM Disconnect");
                addLog("[SYSTEM EVENT] Simulated CGM reconnected.");
            }
            updateStatus();
        });
        connect(occlusionButton, &QPushButton::clicked, this, [this, occlusionButton]() {
            if(!m_cartridge->isOccluded()){
                m_cartridge->setOcclusion(true);
                occlusionButton->setText("Clear Occlusion");
                addLog("[SYSTEM EVENT] Simulated Occlusion detected.");
            } else {
                m_cartridge->setOcclusion(false);
                occlusionButton->setText("Toggle Occlusion");
                addLog("[SYSTEM EVENT] Simulated Occlusion cleared.");
            }
            updateStatus();
        });
        connect(backFromOptions, &QPushButton::clicked, this, [this]() { m_navManager->navigateToHome(); });
        connect(backFromHistory, &QPushButton::clicked, this, [this]() { m_navManager->navigateToHome(); });
        QVBoxLayout* mainLayoutWidget = new QVBoxLayout(this);
        mainLayoutWidget->addWidget(m_mainStackedWidget);
        setLayout(mainLayoutWidget);
        connect(createProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onCreateProfile);
        connect(editProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onEditProfile);
        connect(deleteProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onDeleteProfile);
    }
public slots:
    void updateStatus() {
        batteryBox->setText("Battery\n" + QString::number(m_battery->getStatus()));
        insulinBox->setText("Insulin\n" + QString::number(m_cartridge->getInsulinLevel()));
        iobBox->setText("IOB\n" + QString::number(m_iob->getIOB()));
        cgmBox->setText("CGM\n" + QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
        updateGraph();
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
            // Clear the current profile event log so that only events for the new profile accumulate
            m_logTextEdit->clear();
            addLog("[PROFILE EVENT] Created profile: " + name);
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
            addLog("[PROFILE EVENT] Updated profile: " + QString::fromStdString(m_currentProfile->getName()));
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
            addLog("[PROFILE EVENT] Deleted profile: " + QString::fromStdString(m_currentProfile->getName()));
            m_profileManager->deleteProfile(m_currentProfile->getName());
            m_currentProfile = nullptr;
            updateProfileDisplay();
            // Clear the event log for the now-deleted profile
            m_logTextEdit->clear();
        }
    }
    void onBolus() {
        BolusCalculationDialog dlg(m_currentProfile, m_iob, m_cartridge, m_sensor, this);
        connect(&dlg, &BolusCalculationDialog::mealInfoEntered, this, [this](double newBG) {
            m_sensor->updateGlucoseData(newBG);
            updateStatus();
            QTimer* mealRiseTimer = new QTimer(this);
            connect(mealRiseTimer, &QTimer::timeout, this, [this, mealRiseTimer, newBG]() {
                float currentBG = m_sensor->getGlucoseLevel();
                float targetMealBG = newBG + 2.0f;
                if (currentBG < targetMealBG) {
                    m_sensor->updateGlucoseData(currentBG + 0.5f);
                    updateStatus();
                    addLog(QString("[BOLUS EVENT] Meal Eaten: CGM increased to %1 mmol/L").arg(m_sensor->getGlucoseLevel(), 0, 'f', 1));
                } else {
                    mealRiseTimer->stop();
                    mealRiseTimer->deleteLater();
                    addLog("[BOLUS EVENT] CGM rise finished.");
                }
            });
            mealRiseTimer->start(5000);
        });
        connect(&dlg, &BolusCalculationDialog::extendedBolusParameters, this,
                [this](double duration, double immediateDose, double extendedDose, double ratePerHour) {
                    int totalTicks = static_cast<int>(duration);
                    addLog(QString("[BOLUS EVENT] Starting Extended Bolus Delivery... Immediate: %1 u, Extended: %2 u over %3 hrs at %4 u/hr")
                               .arg(immediateDose)
                               .arg(extendedDose)
                               .arg(totalTicks)
                               .arg(ratePerHour, 0, 'f', 2));
                    QTimer* timer = new QTimer(this);
                    int* tick = new int(0);
                    connect(timer, &QTimer::timeout, this, [=]() mutable {
                        if (*tick < totalTicks) {
                            if (m_iob)
                                m_iob->updateIOB(m_iob->getIOB() + ratePerHour);
                            if (m_cartridge)
                                m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - ratePerHour);
                            updateStatus();
                            addLog(QString("[BOLUS EVENT] %1/%2 hrs | +%3 u delivered (extended)")
                                       .arg(*tick + 1)
                                       .arg(totalTicks)
                                       .arg(ratePerHour, 0, 'f', 2));
                            (*tick)++;
                        } else {
                            timer->stop();
                            timer->deleteLater();
                            delete tick;
                            addLog("[BOLUS EVENT] Extended Bolus Completed");
                        }
                    });
                    timer->start(10000);
                    QTimer* cgmTimer = new QTimer(this);
                    connect(cgmTimer, &QTimer::timeout, this, [=]() {
                        double currentBG = m_sensor->getGlucoseLevel();
                        double targetBG = (m_currentProfile) ? m_currentProfile->getTargetGlucose() : 5.0;
                        if (currentBG > targetBG) {
                            double updated = currentBG - 0.5;
                            if (updated < targetBG)
                                updated = targetBG;
                            m_sensor->updateGlucoseData(updated);
                            updateStatus();
                            addLog(QString("[BOLUS EVENT] CGM: %1 mmol/L").arg(updated, 0, 'f', 2));
                        } else {
                            cgmTimer->stop();
                            cgmTimer->deleteLater();
                            addLog("[BOLUS EVENT] CGM @ Target: Complete");
                        }
                    });
                    cgmTimer->start(10000);
                });
        connect(&dlg, &BolusCalculationDialog::immediateBolusParameters, this, [this](double bolus) {
            if (m_cartridge && m_cartridge->getInsulinLevel() < bolus) {
                QMessageBox::warning(this, "Insufficient Insulin",
                                     "Not enough insulin in the cartridge for this bolus.");
                return;
            }
            if (m_iob)
                m_iob->updateIOB(m_iob->getIOB() + bolus);
            if (m_cartridge) {
                int newLevel = m_cartridge->getInsulinLevel() - static_cast<int>(bolus);
                m_cartridge->updateInsulinLevel(newLevel > 0 ? newLevel : 0);
            }
            if (m_battery)
                m_battery->discharge();
            addLog(QString("[BOLUS EVENT] Immediate Bolus Delivered: %1 u").arg(bolus, 0, 'f', 1));
            updateStatus();
            QTimer* cgmTimer = new QTimer(this);
            connect(cgmTimer, &QTimer::timeout, this, [=]() {
                double currentBG = m_sensor->getGlucoseLevel();
                double targetBG = (m_currentProfile) ? m_currentProfile->getTargetGlucose() : 5.0;
                if (currentBG > targetBG) {
                    double updated = currentBG - 0.5;
                    if (updated < targetBG)
                        updated = targetBG;
                    m_sensor->updateGlucoseData(updated);
                    updateStatus();
                    addLog(QString("[BOLUS EVENT] CGM updated: %1 mmol/L").arg(updated, 0, 'f', 2));
                } else {
                    cgmTimer->stop();
                    cgmTimer->deleteLater();
                    addLog("[BOLUS EVENT] CGM simulation complete.");
                }
            });
            cgmTimer->start(10000);
        });
        dlg.exec();
    }

    // Charging simulation (Use Case 12)
    void onCharge() {
        QPushButton* chargeButton = qobject_cast<QPushButton*>(sender());
        if (!chargeButton)
            return;
        if (m_chargingTimer != nullptr) {
            m_chargingTimer->stop();
            m_chargingTimer->deleteLater();
            m_chargingTimer = nullptr;
            chargeButton->setStyleSheet("");
            addLog("[SYSTEM EVENT] Charging stopped.");
            return;
        }
        chargeButton->setStyleSheet("background-color: green; color: white;");
        addLog("[SYSTEM EVENT] Charging started...");
        m_chargingTimer = new QTimer(this);
        connect(m_chargingTimer, &QTimer::timeout, this, [=]() {
            if (m_battery->getStatus() < 100) {
                m_battery->charge();
                updateStatus();
            } else {
                m_chargingTimer->stop();
                m_chargingTimer->deleteLater();
                m_chargingTimer = nullptr;
                chargeButton->setStyleSheet("");
                addLog("[SYSTEM EVENT] Charging completed.");
            }
        });
        m_chargingTimer->start(1000);
    }


    // Start basal insulin delivery (Use Case 6)
    void startBasalDelivery() {
        if (!m_currentProfile) {
            QMessageBox::warning(this, "Basal Delivery", "No profile loaded.");
            return;
        }
        if (m_battery && m_battery->getStatus() == 0) {
            QMessageBox::warning(this, "Basal Delivery", "Battery is depleted. Please charge the pump.");
            return;
        }
        float rate = m_currentProfile->getBasalRate();
        if (rate <= 0.0f) {
            QMessageBox::warning(this, "Basal Delivery", "Set a valid basal rate in the profile to start delivery.");
            return;
        }


        BasalManager* basalMgr = new BasalManager(m_currentProfile, m_battery, m_cartridge, m_iob, m_sensor, this);
        basalMgr->startBasalDelivery(
            [this](const QString &msg){ addLog(msg); },
            [this](){ updateStatus(); },
            [this](const QString &status){ basalStatusLabel->setText(status); }
            );
    }

    // Adds the profile information in home display
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


    // AddLog() now also logs the event via DataManager.
    void addLog(const QString& message) {
        m_logTextEdit->append(message);
        if(m_dataManager)
            m_dataManager->logEvent(message);
    }



    // updateHistory() changes the History screen from DataManager.
    void updateHistory() {
        if(m_dataManager && m_historyTextEdit)
            m_historyTextEdit->setText(m_dataManager->getHistory());
    }


    // ADDING GRAPHICAL INPUT TO LOOK AT CGM
    void updateGraph() {
        for (int i = 0; i < m_graph_points->count(); i++) {
            QPointF cur = m_graph_points->at(i);
            if (cur.x() < -6) {
                m_graph_points->remove(i);
                continue;
            }
            m_graph_points->replace(i, cur.x() - .5, cur.y());
        }
        m_graph_points->append(QPointF(0, m_sensor->getGlucoseLevel()));
        m_predicted_points->clear();
        if (m_graph_points->count() >= 3) {
            QPointF p0 = m_graph_points->at(m_graph_points->count() - 1);
            QPointF p1 = m_graph_points->at(m_graph_points->count() - 2);
            QPointF p2 = m_graph_points->at(m_graph_points->count() - 3);
            QPointF averageDiff = ((p1 - p0) + (p2 - p1)) * .5;
            m_predicted_points->append(p0 - averageDiff);
            m_predicted_points->append(p0 - 2 * averageDiff);
            m_predicted_points->append(p0 - 3 * averageDiff);
        }
        m_graph_line->clear();
        m_graph_line->append(m_graph_points->points());
        m_graph_line->append(m_predicted_points->points());
    }
private:
    QLabel* createStatusBox(const QString& title, const QString& value) {

        //Set panel
        QLabel* box = new QLabel(title + "\n" + value, this);
        box->setFrameStyle(QFrame::Panel | QFrame::Raised);
        box->setAlignment(Qt::AlignCenter);
        box->setFixedSize(120, 80);

        //PUMP STATUSES IN THE HOME SCREEN
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

    QTextEdit* m_historyTextEdit;
    DataManager* m_dataManager;
    ProfileManager* m_profileManager;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    Profile* m_currentProfile = nullptr;
    QTimer* m_chargingTimer;
    QChart* m_chart;
    QScatterSeries* m_graph_points;
    QScatterSeries* m_predicted_points;
    QSplineSeries* m_graph_line;
    QLabel* basalStatusLabel = nullptr;
    NavigationManager* m_navManager;
    QStackedWidget* m_mainStackedWidget; //Swtich view
};

//--------------------------------------------------------
// PUMP SIMULATOR MAIN WIDGET (Modified for PIN setup)
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


        //PIN set on POWER ON.
        m_powerButton = new QPushButton("POWER ON", this);
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


            // If no PIN is set, get user to set one (Use Case 1)
            if(m_userPIN.isEmpty()){
                SetPinDialog setPinDlg(this);
                if(setPinDlg.exec() == QDialog::Accepted){
                    m_userPIN = setPinDlg.getPin();
                } else {
                    m_powerButton->setChecked(false);
                    return;
                }
            } else {

                //Pin set - Check if pin is correct
                PINDialog pinDlg(m_userPIN, this);
                if(pinDlg.exec() != QDialog::Accepted) {
                    m_powerButton->setChecked(false);
                    return;
                }
            }
            m_pump->powerOn();
            m_powerButton->setText("Power Off");
            m_homeScreen->setVisible(true);
            m_homeScreen->updateStatus(); //Update the indicators to home screen
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
    QString m_userPIN;
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
