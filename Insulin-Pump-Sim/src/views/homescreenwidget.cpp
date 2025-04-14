#include "src/views/homescreenwidget.h"
#include "src/dialogs/newprofiledialog.h"
#include "src/logic/basalmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include <QPainter>
#include <QtCharts/QChartView>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QApplication>
#include "src/dialogs/pindialog.h"
#include "pumpsimulatormainwidget.h"
#include "optionspagecontroller.h"
#include "src/logic/insulindelivery.h"

HomeScreenWidget::HomeScreenWidget(ProfileManager* profileManager,
                                   Battery* battery,
                                   InsulinCartridge* cartridge,
                                   IOB* iob,
                                   CGMSensor* sensor,
                                   QWidget* parent)
    : QWidget(parent),
    m_profileManager(profileManager),
    m_battery(battery),
    m_cartridge(cartridge),
    m_iob(iob),
    m_sensor(sensor),
    m_currentProfile(nullptr),
    m_chargingTimer(nullptr),
    m_basalButton(nullptr),
    m_alertsEnabled(true)
{
    // Event logging
    m_dataManager = new DataManager();


    m_mainStackedWidget = new QStackedWidget(this);
    QWidget* homePage = new QWidget(this);
    QVBoxLayout* homeLayout = new QVBoxLayout(homePage);

    // CGM graph and status layout.
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



    // Status boxes for Battery, Insulin, IOB, and CGM
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
    homeLayout->addWidget(statusGroup);

    // Profile management
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

    // Event log
    QGroupBox* logGroup = new QGroupBox("Event Log", homePage);
    m_logTextEdit = new QTextEdit(homePage);
    m_logTextEdit->setReadOnly(true);
    QVBoxLayout* logLayout = new QVBoxLayout();
    logLayout->addWidget(m_logTextEdit);
    logGroup->setLayout(logLayout);

    QHBoxLayout* profileAndLogLayout = new QHBoxLayout();
    profileAndLogLayout->addWidget(profileGroup);
    profileAndLogLayout->addWidget(logGroup);
    homeLayout->addLayout(profileAndLogLayout);

    // Navigation -> buttons at the bottom
    QPushButton* bolusButton = new QPushButton("Bolus", homePage);
    QPushButton* optionsButton = new QPushButton("Options", homePage);
    QPushButton* historyButton = new QPushButton("History", homePage);
    QPushButton* chargeButton = new QPushButton("Charge", homePage);
    m_basalButton = new QPushButton("Basal Delivery", homePage);
    QPushButton* disconnectButton = new QPushButton("Toggle Disconnects CGM", homePage);
    QPushButton* occlusionButton = new QPushButton("Toggle Occlusion", homePage);
    QHBoxLayout* navLayout = new QHBoxLayout();
    navLayout->addWidget(bolusButton);
    navLayout->addWidget(optionsButton);
    navLayout->addWidget(historyButton);
    navLayout->addWidget(chargeButton);
    navLayout->addWidget(m_basalButton);
    navLayout->addWidget(disconnectButton);
    navLayout->addWidget(occlusionButton);
    QPushButton* crashButton = new QPushButton("Crash Insulin", homePage);
    crashButton->setStyleSheet("background-color: red; color: white;");
    navLayout->addWidget(crashButton);
    homeLayout->addLayout(navLayout);

    // Basal status label ->
    basalStatusLabel = new QLabel("Basal Delivery not started.", homePage);
    basalStatusLabel->setStyleSheet("background-color: lightgreen; padding: 4px;");
    basalStatusLabel->setAlignment(Qt::AlignCenter);
    basalStatusLabel->setFixedHeight(30);
    homeLayout->addWidget(basalStatusLabel);

    // History page
    QWidget* historyPage = new QWidget(this);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyPage);
    m_historyTextEdit = new QTextEdit(historyPage);
    m_historyTextEdit->setReadOnly(true);
    QPushButton* backFromHistory = new QPushButton("Back", historyPage);
    historyLayout->addWidget(m_historyTextEdit);
    historyLayout->addWidget(backFromHistory);

    // Options button
    m_optionsController = new OptionsPageController(this, m_profileManager);
    QWidget* optionsPage = m_optionsController->getWidget();

    // Add pages to the stacked widget: Home (index 0), Options (index 1), History (index 2)
    m_mainStackedWidget->addWidget(homePage);
    m_mainStackedWidget->addWidget(optionsPage);
    m_mainStackedWidget->addWidget(historyPage);

    // navigation to different views
    m_navManager = new NavigationManager(m_mainStackedWidget);
    connect(bolusButton, &QPushButton::clicked, this, &HomeScreenWidget::onBolus);
    connect(optionsButton, &QPushButton::clicked, this, [this]() {
        updateOptionsPage();
        m_navManager->navigateToOptions();
    });
    connect(historyButton, &QPushButton::clicked, this, [this]() { updateHistory(); m_navManager->navigateToHistory(); });
    connect(chargeButton, &QPushButton::clicked, this, &HomeScreenWidget::onCharge);
    connect(m_basalButton, &QPushButton::clicked, this, &HomeScreenWidget::toggleBasalDelivery);
    connect(disconnectButton, &QPushButton::clicked, this, [this, disconnectButton]() {
        if(m_sensor->isConnected()){
            m_sensor->disconnectSensor();
            disconnectButton->setText("Toggle CGM Reconnect");
            addLog("Simulated CGM disconnect.");
        } else {
            m_sensor->connectSensor();
            disconnectButton->setText("Toggle CGM Disconnect");
            addLog("Simulated CGM reconnected.");
        }
        updateStatus();
    });
    connect(occlusionButton, &QPushButton::clicked, this, [this, occlusionButton]() {
        if(!m_cartridge->isOccluded()){
            m_cartridge->setOcclusion(true);
            occlusionButton->setText("Clear Occlusion");
            addLog("Simulated Occlusion detected.");
        } else {
            m_cartridge->setOcclusion(false);
            occlusionButton->setText("Toggle Occlusion");
            addLog("Simulated Occlusion cleared.");
        }
        updateStatus();
    });
    connect(backFromHistory, &QPushButton::clicked, this, [this]() { m_navManager->navigateToHome(); });

    QVBoxLayout* mainLayoutWidget = new QVBoxLayout(this);
    mainLayoutWidget->addWidget(m_mainStackedWidget);
    setLayout(mainLayoutWidget);

    // IOB decay timer
    QTimer* iobDecayTimer = new QTimer(this);
    connect(iobDecayTimer, &QTimer::timeout, this, [this]() {
        if (m_iob && m_iob->isActive()) {
            m_iob->decay();  // silently reduce IOB
            updateStatus();  // update display
        }
    });
    iobDecayTimer->start(20000);  // every 20 seconds

    // Crash button connection
    connect(crashButton, &QPushButton::clicked, this, &HomeScreenWidget::onCrashInsulin);
    connect(createProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onCreateProfile);
    connect(editProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onEditProfile);
    connect(deleteProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onDeleteProfile);

    // OptionsPageController connections like PIN changed and sleep mode
    connect(m_optionsController, &OptionsPageController::alertToggled, this, [this](bool disabled){
        addLog(disabled ? "[ALERT] 🔕 Alerts disabled" : "[ALERT] 🔔 Alerts enabled");
        m_alertsEnabled = !disabled;
    });
    connect(m_optionsController, &OptionsPageController::changePinRequested, this, [this](){
        PINDialog dlg(PinMode::Set, "", this);
        if(dlg.exec() == QDialog::Accepted) {
            QString newPin = dlg.enteredPIN();
            PumpSimulatorMainWidget* pumpSim = qobject_cast<PumpSimulatorMainWidget*>(this->parentWidget());
            if(pumpSim)
                pumpSim->setUserPIN(newPin);
            addLog("[SECURITY] 🔑 PIN changed.");
            QMessageBox::information(this, "PIN Changed", "PIN has been updated.");
        }
    });
    connect(m_optionsController, &OptionsPageController::sleepModeToggled, this, [this](bool enabled, int timeout){
        if(enabled) {
            addLog(QString("[SLEEP MODE] Enabled. Will activate after %1 seconds of inactivity.").arg(timeout));
            QTimer::singleShot(timeout * 1000, this, [this](){
                addLog("[SLEEP MODE] Pump is now in sleep mode.");
                setEnabled(false);
            });
        } else {
            addLog("[SLEEP MODE] Disabled.");
            setEnabled(true);
        }
    });
    connect(m_optionsController, &OptionsPageController::powerOffRequested, this, [this](){
        QMessageBox::information(this, "Powering Off", "Pump is now powered off.");
        qApp->quit();
    });
    connect(m_optionsController, &OptionsPageController::togglePumpRequested, this, [this](){
        PumpSimulatorMainWidget* pumpSim = qobject_cast<PumpSimulatorMainWidget*>(this->parentWidget());
        if(pumpSim) {
            pumpSim->togglePump();
            addLog("[PUMP] Toggled pump power state.");
        }
    });
    connect(m_optionsController, &OptionsPageController::backClicked, this, [this](){
        m_navManager->navigateToHome();
    });

    // Get InsulinDeliveryController
    m_insulinDelivery = new InsulinDelivery(
        m_currentProfile, // Find the HomeScreenWidget’s current profile pointer
        m_battery,
        m_cartridge,
        m_iob,
        m_sensor,
        [this](const QString &msg){ addLog(msg); },
        [this](){ updateStatus(); },
        [this](const QString &status){ basalStatusLabel->setText(status); },
        this
        );
}

void HomeScreenWidget::updateStatus() {
    batteryBox->setText("Battery\n" + QString::number(m_battery->getStatus()));
    insulinBox->setText("Insulin\n" + QString::number(m_cartridge->getInsulinLevel()));
    iobBox->setText("IOB\n" + QString::number(m_iob->getIOB()));
    cgmBox->setText("CGM\n" + QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
    updateGraph();
}


//creates profile
void HomeScreenWidget::onCreateProfile() {
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
        m_logTextEdit->clear();
        addLog("[PROFILE] Created profile: " + name);
    }
}


//edits profile and information
void HomeScreenWidget::onEditProfile() {
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
        addLog("[PROFILE] Updated profile: " + QString::fromStdString(m_currentProfile->getName()));
    }
}

//delete profile
void HomeScreenWidget::onDeleteProfile() {
    if(!m_currentProfile) {
        QMessageBox::warning(this, "Delete Profile", "No profile loaded to delete.");
        return;
    }
    int ret = QMessageBox::question(this, "Delete Profile",
                                    "Are you sure you want to delete the current profile?",
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes) {
        addLog("[PROFILE] Deleted profile: " + QString::fromStdString(m_currentProfile->getName()));
        m_profileManager->deleteProfile(m_currentProfile->getName());
        m_currentProfile = nullptr;
        updateProfileDisplay();
        m_logTextEdit->clear();
    }
}


// update the bolus dialog
void HomeScreenWidget::onBolus() {
    m_insulinDelivery->launchBolusDialog(this);
}


//check the charge
void HomeScreenWidget::onCharge() {
    QPushButton* chargeButton = qobject_cast<QPushButton*>(sender());
    if (!chargeButton)
        return;
    if (m_chargingTimer != nullptr) {
        m_chargingTimer->stop();
        m_chargingTimer->deleteLater();
        m_chargingTimer = nullptr;
        chargeButton->setStyleSheet("");
        addLog("[SYSTEM] ⚡ Charging stopped.");
        return;
    }
    chargeButton->setStyleSheet("background-color: green; color: white;");
    addLog("[SYSTEM] 🔌 Charging started...");
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
            addLog("[SYSTEM] 🔋 Charging completed.");
        }
    });
    m_chargingTimer->start(1000);
}


//get basal delivery -> stop start reums pause
void HomeScreenWidget::toggleBasalDelivery() {
    if (!m_currentProfile) {
        QMessageBox::warning(this, "Basal Delivery", "No profile loaded.");
        return;
    }
    m_insulinDelivery->toggleBasalDelivery();
}

void HomeScreenWidget::startBasalDelivery() {
    m_insulinDelivery->startBasalDelivery();
}

void HomeScreenWidget::updateProfileDisplay() {
    if(m_currentProfile) {
        currentProfileLabel->setText(
            "Current Profile: \"" + QString::fromStdString(m_currentProfile->getName()) + "\"\n"
                                                                                          "   - Basal Rate: " + QString::number(m_currentProfile->getBasalRate()) + " u/hr\n"
                                                                  "   - Carb Ratio: 1u per " + QString::number(static_cast<int>(m_currentProfile->getCarbRatio())) + " g\n"
                                                                                    "   - Correction Factor: 1u per " + QString::number(static_cast<int>(m_currentProfile->getCorrectionFactor())) + " mmol/L\n"
                                                                                           "   - Target BG: " + QString::number(m_currentProfile->getTargetGlucose()) + " mmol/L"
            );
        m_insulinDelivery->setCurrentProfile(m_currentProfile);
    } else {
        currentProfileLabel->setText("No profile loaded.");
    }
}


//add to the history logging
void HomeScreenWidget::updateHistory() {
    if(m_dataManager && m_historyTextEdit)
        m_historyTextEdit->setText(m_dataManager->getHistory());
}

//graph
void HomeScreenWidget::updateGraph() {
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


//PUMP STATUS
QLabel* HomeScreenWidget::createStatusBox(const QString& title, const QString& value) {
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

//
void HomeScreenWidget::updateOptionsPage() {
    m_optionsController->updateProfileSwitching(m_profileManager->getProfiles(), m_currentProfile,
                                                [this](const Profile& profile) {
                                                    m_currentProfile = m_profileManager->selectProfile(profile.getName());
                                                    updateProfileDisplay();
                                                    addLog("[PROFILE] Switched to profile: " + QString::fromStdString(profile.getName()));
                                                    m_navManager->navigateToHome();
                                                }
                                                );
}


//simualation to crash insulin
void HomeScreenWidget::onCrashInsulin() {
    addLog("[SYSTEM]: ❌ Crash -> Stopping all insulin delivery");
    m_insulinDelivery->stopAllDelivery();
    m_basalButton->setText("Start Basal Delivery");
    basalStatusLabel->setText("Basal stopped (System Crash)");
    QMessageBox::critical(this, "System Crash", "All insulin delivery has been stopped due to a critical error.");
}



//adding the logs
void HomeScreenWidget::addLog(const QString& message) {
    if(m_alertsEnabled)
        m_logTextEdit->append(message);
    if(m_dataManager)
        m_dataManager->logEvent(message);
}
