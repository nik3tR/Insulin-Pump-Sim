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
#include <iostream>
#include <string>
#include <vector>

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
    Battery() : level(75) {} // For testing -> setting to 75% to test charge

    void discharge() { if(level > 0) level -= 10; }
    int getStatus() const { return level; }
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
    CGMSensor() : currentGlucoseLevel(5.5f) {} //Default
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
};

//--------------------------------------------------------
// NEW PROFILE DIALOG
//--------------------------------------------------------
#include <QObject>

class NewProfileDialog : public QDialog {
    Q_OBJECT
public:
    NewProfileDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Create New Profile");
        QFormLayout* formLayout = new QFormLayout(this);

        nameEdit = new QLineEdit(this);
        basalEdit = new QLineEdit(this);
        carbEdit = new QLineEdit(this);
        correctionEdit = new QLineEdit(this);
        targetEdit = new QLineEdit(this);

        formLayout->addRow("Name:", nameEdit);
        formLayout->addRow("Basal Rate (U/hr):", basalEdit);
        formLayout->addRow("Carb Ratio (1:X):", carbEdit);
        formLayout->addRow("Correction Factor (1:X):", correctionEdit);
        formLayout->addRow("Target BG (mg/dL):", targetEdit);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                               | QDialogButtonBox::Cancel,
                                                           this);
        formLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &NewProfileDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &NewProfileDialog::reject);
    }

    QString getName() const { return nameEdit->text(); }
    float getBasalRate() const { return basalEdit->text().toFloat(); }
    float getCarbRatio() const { return carbEdit->text().toFloat(); }
    float getCorrectionFactor() const { return correctionEdit->text().toFloat(); }
    float getTargetGlucose() const { return targetEdit->text().toFloat(); }

private:
    QLineEdit *nameEdit, *basalEdit, *carbEdit, *correctionEdit, *targetEdit;
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

        // Close button only; charging happens automatically.
        QPushButton* closeButton = new QPushButton("Close", this);
        layout->addWidget(batteryLabel);
        layout->addWidget(closeButton);
        setLayout(layout);

        // Timer to increment battery level by 0.5 seconds%
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ChargingDisplayDialog::onTimeout);
        m_timer->start(500);

        connect(closeButton, &QPushButton::clicked, this, &ChargingDisplayDialog::accept);
    }

public slots:
    void onTimeout() {
        if(m_battery->level < 100) {
            m_battery->level++; // Increment battery level by 1
            batteryLabel->setText("Battery Level: " + QString::number(m_battery->getStatus()));
            std::cout << "[ChargingDisplayDialog] Battery level: " << m_battery->getStatus() << "\n";
        } else {
            m_timer->stop(); // Stop once fully charged
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
        // ----- Pump Status Section -----
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

        // ----- Personal Profiles Section -----
        QGroupBox* profileGroup = new QGroupBox("Personal Profiles", this);
        currentProfileLabel = new QLabel("No profile loaded.", this);
        QPushButton* createProfileButton = new QPushButton("Create New Profile", this);
        QVBoxLayout* profileLayout = new QVBoxLayout();
        profileLayout->addWidget(currentProfileLabel);
        profileLayout->addWidget(createProfileButton);
        profileGroup->setLayout(profileLayout);

        connect(createProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onCreateProfile);

        // ----- Navigation Buttons -----
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

        // ----- Main Layout -----
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(statusGroup);
        mainLayout->addWidget(profileGroup);
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

    void onBolus() { std::cout << "[HomeScreenWidget] Bolus button clicked.\n"; }
    void onOptions() { std::cout << "[HomeScreenWidget] Options button clicked.\n"; }
    void onHistory() { std::cout << "[HomeScreenWidget] History button clicked.\n"; }

    // Incremental Charging
    void onCharge() {
        // Start incremental charging via the dialog:
        ChargingDisplayDialog dlg(m_battery, this);
        dlg.exec();
        updateStatus();
    }

    void updateProfileDisplay() {
        if(m_currentProfile) {
            currentProfileLabel->setText(
                "Current Profile: \"" + QString::fromStdString(m_currentProfile->getName()) + "\"\n"
                                                                                              "   - Basal Rate: " + QString::number(m_currentProfile->getBasalRate()) + " U/hr\n"
                                                                      "   - Carb Ratio: 1:" + QString::number(static_cast<int>(m_currentProfile->getCarbRatio())) + "\n"
                                                                                        "   - Correction Factor: 1:" + QString::number(static_cast<int>(m_currentProfile->getCorrectionFactor())) + "\n"
                                                                                               "   - Target BG: " + QString::number(m_currentProfile->getTargetGlucose()) + " mg/dL"
                );
        } else {
            currentProfileLabel->setText("No profile loaded.");
        }
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
