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

class Profile; // forward declaration

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
    Battery() : level(100) {} // For testing
    void discharge() { if(level > 0) level -= 10; if (level < 0) level = 0; }
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
// MODULAR CLASSES FOR INSULIN DELIVERY
//--------------------------------------------------------

// ControlIQ Module: adjusts delivery based on CGM reading
class ControlIQ {
public:
    ControlIQ() {}
    double adjustDelivery(double cgm) {
        // Simplistic adjustment: if BG is high, full rate; if low, reduce rate.
        return (cgm >= 7.0 ? 1.0 : 0.5);
    }
};

// BolusManager: computes bolus values based on meal info and profile
class BolusManager {
public:
    BolusManager(Profile* profile, IOB* iob) : m_profile(profile), m_iob(iob) {}

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

    // Computes immediate and extended doses given a percentage for the immediate bolus
    void computeExtendedBolus(double finalBolus, double immediatePercentage, double &immediateDose, double &extendedDose) {
        immediateDose = finalBolus * immediatePercentage / 100.0;
        extendedDose = finalBolus - immediateDose;
    }

private:
    Profile* m_profile;
    IOB* m_iob;
};

// BasalManager: handles the periodic delivery of basal insulin
#include <functional>
class BasalManager : public QObject {
    Q_OBJECT
public:
    BasalManager(Profile* profile, Battery* battery, InsulinCartridge* cartridge, IOB* iob, CGMSensor* sensor, QObject* parent = nullptr)
        : QObject(parent), m_profile(profile), m_battery(battery), m_cartridge(cartridge), m_iob(iob), m_sensor(sensor) {}

    // The callbacks allow the UI to log events, update status, and change the basal status label.
    void startBasalDelivery(std::function<void(const QString&)> logCallback,
                            std::function<void()> updateStatusCallback,
                            std::function<void(const QString&)> basalStatusCallback) {
        if (!m_profile) {
            logCallback("No profile loaded.");
            return;
        }
        if (m_battery && m_battery->getStatus() == 0) {
            logCallback("Battery is depleted. Please charge the pump.");
            return;
        }
        float rate = m_profile->getBasalRate();
        if (rate <= 0.0f) {
            logCallback("Set a valid basal rate in the profile to start delivery.");
            return;
        }
        logCallback(QString("Basal Delivery started at %1 u/hr").arg(rate));

        m_timer = new QTimer(this);
        ControlIQ controlIQ;
        connect(m_timer, &QTimer::timeout, this, [=]() mutable {
            if (m_battery && m_battery->getStatus() == 0) {
                logCallback("🛑 Battery drained. Basal Delivery stopped.");
                basalStatusCallback("Basal Delivery Stopped (Battery 0%)");
                m_timer->stop();
                m_timer->deleteLater();
                return;
            }
            if (m_battery->getStatus() <= 15 && m_battery->getStatus() > 0) {
                logCallback("⚠️ Low Battery Warning: Battery is low (15%).");
            }
            float cgm = m_sensor->getGlucoseLevel();
            // Use ControlIQ to adjust the basal rate based on CGM reading
            double adjustment = controlIQ.adjustDelivery(cgm);
            float adjustedRate = rate * adjustment;

            // Pause basal delivery if CGM is too low
            if (cgm < 4.0f) {
                basalStatusCallback("Basal Paused (Low CGM)");
                logCallback("⚠️ Basal Delivery Paused — CGM too low (< 3.9 mmol/L)");
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
            logCallback(QString("Basal Delivered: %1 u | CGM: %2 mmol/L")
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
// PIN DIALOG (Security)
//--------------------------------------------------------
class PINDialog : public QDialog {
    Q_OBJECT
public:
    PINDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Enter PIN");
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Please enter your 4-digit PIN:", this);
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
        // Hardcoded PIN for simulation
        if(pinEdit->text() == "1234")
            accept();
        else {
            QMessageBox::warning(this, "Invalid PIN", "The PIN you entered is incorrect.");
            pinEdit->clear();
        }
    }
private:
    QLineEdit* pinEdit;
};

//--------------------------------------------------------
// NAVIGATION MANAGER (Navigation)
//--------------------------------------------------------
// Modified: now accepts a pointer to a QStackedWidget to perform screen switching.
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
            // Bolus remains as a dialog in this design.
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
// BOLUS CALCULATION DIALOG (with Extended Bolus Simulation)
//--------------------------------------------------------
class BolusCalculationDialog : public QDialog {
    Q_OBJECT
public:
    // Constructor receives profile, IOB, and cartridge pointers.
    BolusCalculationDialog(Profile* profile = nullptr, IOB* iob = nullptr, InsulinCartridge* cartridge = nullptr, CGMSensor* sensor = nullptr, QWidget* parent = nullptr)
        : QDialog(parent), m_profile(profile), m_iob(iob), m_cartridge(cartridge), m_sensor(sensor), m_finalBolus(0.0)
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

        // auto-populate blood glucose
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

        // Page 2: Result and Method Selection
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

        // Immediately update IOB and insulin cartridge for immediate bolus
        connect(manualButton, &QPushButton::clicked, this, [this]() {
            std::cout << "[BolusCalculationDialog] Manual Bolus selected.\n";
            emit immediateBolusParameters(m_finalBolus);
            accept();
        });

        // extended dose: Immediately apply specified immediate dose
        connect(extendedButton, &QPushButton::clicked, this, [this]() {
            ExtendedBolusDialog extDlg(this);
            if(extDlg.exec() == QDialog::Accepted) {
                double duration = extDlg.getDuration();
                double immediatePct = extDlg.getImmediatePercentage();
                double immediateDose, extendedDose;
                // Use BolusManager to compute extended bolus parameters
                BolusManager manager(m_profile, m_iob);
                manager.computeExtendedBolus(m_finalBolus, immediatePct, immediateDose, extendedDose);
                double currentRate = (duration > 0) ? extendedDose / duration : 0.0;

                // update IOB and cartridge for immediate dose
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
    // signal the extended bolus
    void extendedBolusParameters(double duration, double immediateDose, double extendedDose, double ratePerHour);
    // signal immediate bolus
    void immediateBolusParameters(double immediateDose);
    // pass the user-entered Current BG update CGM sensor
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

        // Use BolusManager to compute the final bolus based on the inputs.
        BolusManager manager(m_profile, m_iob);
        m_finalBolus = manager.computeFinalBolus(carbs, currentBG);

        QString resultText;
        resultText += "--- BOLUS RESULT ---\n";
        resultText += QString("Carbs: %1g | BG: %2 mmol/L | IOB: %3 u\n\n")
                          .arg(carbs)
                          .arg(currentBG)
                          .arg((m_iob) ? m_iob->getIOB() : 0.0);
        // For display, compute individual components as per the original formulas.
        double carbRatio = (m_profile) ? m_profile->getCarbRatio() : 10.0;
        double correctionFactor = (m_profile) ? m_profile->getCorrectionFactor() : 2.0;
        double targetBG = (m_profile) ? m_profile->getTargetGlucose() : 6.0;
        double carbBolus = carbs / carbRatio;
        double correctionBolus = (currentBG > targetBG) ? (currentBG - targetBG) / correctionFactor : 0.0;
        double totalBolus = carbBolus + correctionBolus;
        double iob = (m_iob) ? m_iob->getIOB() : 0.0;

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
// CHARGING DISPLAY DIALOG (unchanged, but no longer invoked)
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
// Modified: Added a QStackedWidget (m_mainStackedWidget) to manage multiple views (Home, Options, History)
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
        // Create the main stacked widget for screen navigation.
        m_mainStackedWidget = new QStackedWidget(this);

        // --- Home Page ---
        QWidget* homePage = new QWidget(this);
        QVBoxLayout* homeLayout = new QVBoxLayout(homePage);

        // Pump Status Section
        QVBoxLayout* statusLayout = new QVBoxLayout();

        // Graph Subsection
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

        // Personal Profiles Section
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

        // Event Log Section
        QGroupBox* logGroup = new QGroupBox("Event Log", homePage);
        m_logTextEdit = new QTextEdit(homePage);
        m_logTextEdit->setReadOnly(true);
        QVBoxLayout* logLayout = new QVBoxLayout();
        logLayout->addWidget(m_logTextEdit);
        logGroup->setLayout(logLayout);

        // Layout for Profiles and Log side by side
        QHBoxLayout* profileAndLogLayout = new QHBoxLayout();
        profileAndLogLayout->addWidget(profileGroup);
        profileAndLogLayout->addWidget(logGroup);

        // Navigation Buttons
        QPushButton* bolusButton = new QPushButton("Bolus", homePage);
        QPushButton* optionsButton = new QPushButton("Options", homePage);
        QPushButton* historyButton = new QPushButton("History", homePage);
        QPushButton* chargeButton = new QPushButton("Charge", homePage);
        QPushButton* basalButton = new QPushButton("Start Basal Delivery", homePage);
        QHBoxLayout* navLayout = new QHBoxLayout();
        navLayout->addWidget(bolusButton);
        navLayout->addWidget(optionsButton);
        navLayout->addWidget(historyButton);
        navLayout->addWidget(chargeButton);
        navLayout->addWidget(basalButton);

        // Assemble Home Page Layout
        homeLayout->addWidget(statusGroup);
        homeLayout->addLayout(profileAndLogLayout);
        homeLayout->addLayout(navLayout);
        basalStatusLabel = new QLabel("Basal Delivery not started.", homePage);
        basalStatusLabel->setStyleSheet("background-color: lightgreen; padding: 4px;");
        basalStatusLabel->setAlignment(Qt::AlignCenter);
        basalStatusLabel->setFixedHeight(30);
        homeLayout->addWidget(basalStatusLabel);

        // --- Options Page ---
        QWidget* optionsPage = new QWidget(this);
        QVBoxLayout* optionsLayout = new QVBoxLayout(optionsPage);
        QLabel* optionsLabel = new QLabel("Options Screen", optionsPage);
        QPushButton* backFromOptions = new QPushButton("Back", optionsPage);
        optionsLayout->addWidget(optionsLabel);
        optionsLayout->addWidget(backFromOptions);

        // --- History Page ---
        QWidget* historyPage = new QWidget(this);
        QVBoxLayout* historyLayout = new QVBoxLayout(historyPage);
        QLabel* historyLabel = new QLabel("History Screen", historyPage);
        QPushButton* backFromHistory = new QPushButton("Back", historyPage);
        historyLayout->addWidget(historyLabel);
        historyLayout->addWidget(backFromHistory);

        // Add pages to the main stacked widget
        m_mainStackedWidget->addWidget(homePage);     // index 0: Home
        m_mainStackedWidget->addWidget(optionsPage);    // index 1: Options
        m_mainStackedWidget->addWidget(historyPage);      // index 2: History

        // Create a NavigationManager instance with the stacked widget.
        m_navManager = new NavigationManager(m_mainStackedWidget);

        // Connect navigation buttons
        connect(bolusButton, &QPushButton::clicked, this, &HomeScreenWidget::onBolus);
        connect(optionsButton, &QPushButton::clicked, this, [this]() {
            m_navManager->navigateToOptions();
        });
        connect(historyButton, &QPushButton::clicked, this, [this]() {
            m_navManager->navigateToHistory();
        });
        connect(chargeButton, &QPushButton::clicked, this, &HomeScreenWidget::onCharge);
        connect(basalButton, &QPushButton::clicked, this, &HomeScreenWidget::startBasalDelivery);

        // Back buttons for Options and History pages to return to Home.
        connect(backFromOptions, &QPushButton::clicked, this, [this]() {
            m_navManager->navigateToHome();
        });
        connect(backFromHistory, &QPushButton::clicked, this, [this]() {
            m_navManager->navigateToHome();
        });

        // Set the main layout for the HomeScreenWidget.
        QVBoxLayout* mainLayoutWidget = new QVBoxLayout(this);
        mainLayoutWidget->addWidget(m_mainStackedWidget);
        setLayout(mainLayoutWidget);

        // Connections for profile management
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
        // For Bolus, we still use the dialog.
        BolusCalculationDialog dlg(m_currentProfile, m_iob, m_cartridge, m_sensor, this);

        connect(&dlg, &BolusCalculationDialog::mealInfoEntered, this, [this](double newBG) {
            m_sensor->updateGlucoseData(newBG);
            updateStatus();
            QTimer* mealRiseTimer = new QTimer(this);
            connect(mealRiseTimer, &QTimer::timeout, this, [this, mealRiseTimer, newBG]() {
                float currentBG = m_sensor->getGlucoseLevel();
                float targetMealBG = newBG + 2.0f; // simulate an increase of 2 mmol/L post-meal
                if (currentBG < targetMealBG) {
                    m_sensor->updateGlucoseData(currentBG + 0.5f);
                    updateStatus();
                    addLog(QString("📈 Meal Eaten: CGM increased to %1 mmol/L").arg(m_sensor->getGlucoseLevel(), 0, 'f', 1));
                } else {
                    mealRiseTimer->stop();
                    mealRiseTimer->deleteLater();
                    addLog(" ✅ CGM rise finished.");
                }
            });
            mealRiseTimer->start(5000);
        });

        connect(&dlg, &BolusCalculationDialog::extendedBolusParameters, this,
                [this](double duration, double immediateDose, double extendedDose, double ratePerHour) {
                    int totalTicks = static_cast<int>(duration);
                    addLog(QString("🔄 Starting Extended Bolus Delivery...\nImmediate: %1 u \nExtended: %2 u over %3 hrs at %4 u/hr")
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
                            addLog(QString("⏱️ %1/%2 hrs | +%3 u delivered (extended)")
                                       .arg(*tick + 1)
                                       .arg(totalTicks)
                                       .arg(ratePerHour, 0, 'f', 2));
                            (*tick)++;
                        } else {
                            timer->stop();
                            timer->deleteLater();
                            delete tick;
                            addLog("Extended Bolus Completed");
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
                            addLog(QString("📈 CGM: %1 mmol/L").arg(updated, 0, 'f', 2));
                        } else {
                            cgmTimer->stop();
                            cgmTimer->deleteLater();
                            addLog(" ✅ CGM @ Target: Complete");
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
            addLog(QString("Immediate Bolus Delivered: %1 u").arg(bolus, 0, 'f', 1));
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
                    addLog(QString("💉 CGM updated: %1 mmol/L").arg(updated, 0, 'f', 2));
                } else {
                    cgmTimer->stop();
                    cgmTimer->deleteLater();
                    addLog("CGM simulation complete.");
                }
            });
            cgmTimer->start(10000);
        });

        dlg.exec();
    }

    void onCharge() {
        QPushButton* chargeButton = qobject_cast<QPushButton*>(sender());
        if (!chargeButton)
            return;
        if (m_chargingTimer != nullptr) {
            m_chargingTimer->stop();
            m_chargingTimer->deleteLater();
            m_chargingTimer = nullptr;
            chargeButton->setStyleSheet("");
            addLog("Charging stopped.");
            return;
        }
        chargeButton->setStyleSheet("background-color: green; color: white;");
        addLog("Charging started...");
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
                addLog("✅ Charging completed.");
            }
        });
        m_chargingTimer->start(1000);
    }

    // Use BasalManager for starting basal delivery.
    void startBasalDelivery() {
        if (!m_currentProfile) {
            QMessageBox::warning(this, "Basal Delivery", "No profile loaded.");
            return;
        }
        // Check battery level before starting basal delivery.
        if (m_battery && m_battery->getStatus() == 0) {
            QMessageBox::warning(this, "Basal Delivery", "Battery is depleted. Please charge the pump.");
            return;
        }
        float rate = m_currentProfile->getBasalRate();
        if (rate <= 0.0f) {
            QMessageBox::warning(this, "Basal Delivery", "Set a valid basal rate in the profile to start delivery.");
            return;
        }
        // Create a BasalManager instance and start delivery.
        BasalManager* basalMgr = new BasalManager(m_currentProfile, m_battery, m_cartridge, m_iob, m_sensor, this);
        basalMgr->startBasalDelivery(
            [this](const QString &msg){ addLog(msg); },
            [this](){ updateStatus(); },
            [this](const QString &status){ basalStatusLabel->setText(status); }
            );
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

    void addLog(const QString& message) {
        m_logTextEdit->append(message);
    }

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
    QTimer* m_chargingTimer;
    QChart* m_chart;
    QScatterSeries* m_graph_points;
    QScatterSeries* m_predicted_points;
    QSplineSeries* m_graph_line;
    QLabel* basalStatusLabel = nullptr;
    NavigationManager* m_navManager;
    QStackedWidget* m_mainStackedWidget; // For view switching
};

//--------------------------------------------------------
// MAIN SIMULATOR WIDGET: POWER ON/OFF TOGGLE (with PIN-based lock)
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
            // Require PIN entry before powering on
            PINDialog pinDlg(this);
            if(pinDlg.exec() != QDialog::Accepted) {
                // If PIN entry fails, reset button and do not power on.
                m_powerButton->setChecked(false);
                return;
            }
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
