#include "homescreenwidget.h"

// QT_CHARTS_USE_NAMESPACE

// --- Constructor ---
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
      m_chargingTimer(nullptr)
{
    m_dataManager = new DataManager();
    m_mainStackedWidget = new QStackedWidget(this);
    QWidget* homePage = new QWidget(this);
    QVBoxLayout* homeLayout = new QVBoxLayout(homePage);

    // create graph
    m_graphWidget = new GraphWidget(homePage);
    QVBoxLayout* statusLayout = new QVBoxLayout();
    statusLayout->addWidget(m_graphWidget);

    // create battery, insulin, iob, cgm status boxes
    QHBoxLayout* boxLayout = new QHBoxLayout();
    batteryBox = createStatusBox("Battery", QString::number(m_battery->getStatus()));
    insulinBox = createStatusBox("Insulin", QString::number(m_cartridge->getInsulinLevel()));
    iobBox     = createStatusBox("IOB", QString::number(m_iob->getIOB()));
    cgmBox     = createStatusBox("CGM", QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
    boxLayout->addWidget(batteryBox); boxLayout->addWidget(insulinBox);
    boxLayout->addWidget(iobBox);     boxLayout->addWidget(cgmBox);
    QFrame* boxFrame = new QFrame(); boxFrame->setLayout(boxLayout);
    statusLayout->addWidget(boxFrame);
    QGroupBox* statusGroup = new QGroupBox("Pump Status", homePage);
    statusGroup->setLayout(statusLayout);

    // Profile Section
    QGroupBox* profileGroup = new QGroupBox("Personal Profiles", homePage);
    currentProfileLabel = new QLabel("No profile loaded.", homePage);
    QPushButton* createProfileButton = new QPushButton("Create New Profile", homePage);
    QPushButton* editProfileButton   = new QPushButton("Edit Profile", homePage);
    QPushButton* deleteProfileButton = new QPushButton("Delete Profile", homePage);
    QVBoxLayout* profileLayout = new QVBoxLayout();
    profileLayout->addWidget(currentProfileLabel);
    profileLayout->addWidget(createProfileButton);
    profileLayout->addWidget(editProfileButton);
    profileLayout->addWidget(deleteProfileButton);
    profileGroup->setLayout(profileLayout);

    // Log Panel
    QGroupBox* logGroup = new QGroupBox("Event Log", homePage);
    m_logTextEdit = new QTextEdit(homePage); m_logTextEdit->setReadOnly(true);
    QVBoxLayout* logLayout = new QVBoxLayout(); logLayout->addWidget(m_logTextEdit);
    logGroup->setLayout(logLayout);
    QHBoxLayout* profileAndLog = new QHBoxLayout();
    profileAndLog->addWidget(profileGroup); profileAndLog->addWidget(logGroup);

    // Nav Buttons
    QPushButton* bolusButton     = new QPushButton("Bolus", homePage);
    QPushButton* optionsButton   = new QPushButton("Options", homePage);
    QPushButton* historyButton   = new QPushButton("History", homePage);
    QPushButton* chargeButton    = new QPushButton("Charge", homePage);
    QPushButton* basalButton     = new QPushButton("Start Basal Delivery", homePage);
    QPushButton* disconnectBtn   = new QPushButton("Toggle Disconnects CGM", homePage);
    QPushButton* occlusionBtn    = new QPushButton("Toggle Occlusion", homePage);
    QPushButton* pauseSimButton = new QPushButton("Pause Simulation", homePage);

    QHBoxLayout* navLayout = new QHBoxLayout();
    navLayout->addWidget(bolusButton); navLayout->addWidget(optionsButton);
    navLayout->addWidget(historyButton); navLayout->addWidget(chargeButton);
    navLayout->addWidget(basalButton); navLayout->addWidget(disconnectBtn);
    navLayout->addWidget(occlusionBtn);
    navLayout->addWidget(pauseSimButton);

    // Basal Label
    basalStatusLabel = new QLabel("Basal Delivery not started.", homePage);
    basalStatusLabel->setStyleSheet("background-color: lightgreen; padding: 4px;");
    basalStatusLabel->setAlignment(Qt::AlignCenter);
    basalStatusLabel->setFixedHeight(30);

    homeLayout->addWidget(statusGroup);
    homeLayout->addLayout(profileAndLog);
    homeLayout->addLayout(navLayout);
    homeLayout->addWidget(basalStatusLabel);

    // ------------
    // Options Page
    // ------------
    QWidget* optionsPage = new QWidget(this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsPage);
    optionsLayout->setContentsMargins(10, 10, 10, 10);
    optionsLayout->setSpacing(10);

    // Create and style the title label
    QLabel* titleLabel = new QLabel("Current Profile:", optionsPage);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    optionsLayout->addWidget(titleLabel);

    // create and fill the profile selector box
    QComboBox* profileSelector = new QComboBox(optionsPage);
    refreshProfileList(profileSelector);
    optionsLayout->addWidget(profileSelector);
    optionsLayout->addStretch();

    QPushButton* backFromOptions = new QPushButton("Back", optionsPage);
    optionsLayout->addWidget(backFromOptions);

    // History Page
    QWidget* historyPage = new QWidget(this);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyPage);
    m_historyTextEdit = new QTextEdit(historyPage); m_historyTextEdit->setReadOnly(true);
    QPushButton* backFromHistory = new QPushButton("Back", historyPage);
    historyLayout->addWidget(m_historyTextEdit);
    historyLayout->addWidget(backFromHistory);

    m_mainStackedWidget->addWidget(homePage);    // 0
    m_mainStackedWidget->addWidget(optionsPage); // 1
    m_mainStackedWidget->addWidget(historyPage); // 2

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_mainStackedWidget);
    setLayout(mainLayout);

    m_navManager = new NavigationManager(m_mainStackedWidget);

    // --- Connections ---
    connect(bolusButton, &QPushButton::clicked, this, &HomeScreenWidget::onBolus);
    connect(historyButton, &QPushButton::clicked, this, [=](){ updateHistory(); m_navManager->navigateToHistory(); });
    connect(backFromOptions, &QPushButton::clicked, this, [=](){ m_navManager->navigateToHome(); });
    connect(backFromHistory, &QPushButton::clicked, this, [=](){ m_navManager->navigateToHome(); });
    connect(createProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onCreateProfile);
    connect(editProfileButton,   &QPushButton::clicked, this, &HomeScreenWidget::onEditProfile);
    connect(deleteProfileButton, &QPushButton::clicked, this, &HomeScreenWidget::onDeleteProfile);
    connect(chargeButton,        &QPushButton::clicked, this, &HomeScreenWidget::onCharge);
    connect(basalButton,         &QPushButton::clicked, this, &HomeScreenWidget::startBasalDelivery);

    connect(optionsButton, &QPushButton::clicked, this, [=](){
        refreshProfileList(profileSelector);
        m_navManager->navigateToOptions(); }
    );

    // connect profileSelector to the current profile
    connect(profileSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this, profileSelector](int index) {
            if (index >= 0) {
                // Extract the profile name
                QString profileNameQString = profileSelector->itemData(index).toString();
                std::string profileName = profileNameQString.toStdString();

                // Use selectProfile to get the Profile* and update m_currentProfile
                m_currentProfile = m_profileManager->selectProfile(profileName);

                if (m_currentProfile) {
                    updateProfileDisplay();
                    addLog(QString("[PROFILE EVENT] Switched to profile: %1")
                           .arg(profileNameQString));
                }
            }
        });

    connect(disconnectBtn, &QPushButton::clicked, this, [=]() {
        m_sensor->isConnected() ? m_sensor->disconnectSensor() : m_sensor->connectSensor();
        updateStatus();
        addLog(QString("[SIMULATION] CGM %1").arg(m_sensor->isConnected() ? "connected" : "disconnected"));
    });

    connect(occlusionBtn, &QPushButton::clicked, this, [=]() {
        m_cartridge->setOcclusion(!m_cartridge->isOccluded());
        updateStatus();
        addLog(QString("[SIMULATION] Occlusion %1").arg(m_cartridge->isOccluded() ? "enabled" : "cleared"));
    });

    connect(pauseSimButton, &QPushButton::clicked, this, [=]() {
        m_simulationPaused = !m_simulationPaused;

        if (m_simulationPaused) {
            pauseSimButton->setText("Resume Simulation");
            basalStatusLabel->setText("Simulation paused.");
            addLog("[SYSTEM] Simulation paused.");
        } else {
            pauseSimButton->setText("Pause Simulation");
            basalStatusLabel->setText("Simulation resumed.");
            addLog("[SYSTEM] Simulation resumed.");
        }
    });
}

//--------------------------------------------------------
// Updates display indicators for battery, insulin, IOB, CGM
//--------------------------------------------------------
void HomeScreenWidget::updateStatus() {
    batteryBox->setText("Battery\n" + QString::number(m_battery->getStatus()));
    insulinBox->setText("Insulin\n" + QString::number(m_cartridge->getInsulinLevel()));
    iobBox->setText("IOB\n" + QString::number(m_iob->getIOB()));
    cgmBox->setText("CGM\n" + QString::number(m_sensor->getGlucoseLevel()) + " mmol/L");
    updateGraph();

    // resumes delibvery once cgm is at a safe range
    if (m_basalPaused && m_sensor->getGlucoseLevel() >= 4.1f) {
        m_basalPaused = false;
        basalStatusLabel->setText("Basal resumed: CGM safe");
        addLog("[CONTROL IQ] Basal resumed.");
    }
}

//--------------------------------------------------------
// Add log entry (with timestamp via DataManager)
//--------------------------------------------------------
void HomeScreenWidget::addLog(const QString& message) {
    m_logTextEdit->append(message);
    if (m_dataManager)
        m_dataManager->logEvent(message);
}

//--------------------------------------------------------
// Populate History screen with full event log
//--------------------------------------------------------
void HomeScreenWidget::updateHistory() {
    if (m_dataManager && m_historyTextEdit)
        m_historyTextEdit->setText(m_dataManager->getHistory());
}

//--------------------------------------------------------
// Create a styled QLabel for a status box
//--------------------------------------------------------
QLabel* HomeScreenWidget::createStatusBox(const QString& title, const QString& value) {
    QLabel* box = new QLabel(title + "\n" + value, this);
    box->setFrameStyle(QFrame::Panel | QFrame::Raised);
    box->setAlignment(Qt::AlignCenter);
    box->setFixedSize(120, 80);

    if (title == "Battery")
        box->setStyleSheet("background-color: #ADD8E6; color: black;");
    else if (title == "Insulin")
        box->setStyleSheet("background-color: #87CEFA; color: black;");
    else if (title == "IOB")
        box->setStyleSheet("background-color: #B0E0E6; color: black;");
    else if (title == "CGM")
        box->setStyleSheet("background-color: #AFEEEE; color: black;");

    return box;
}

//--------------------------------------------------------
// Profile: Create new profile
//--------------------------------------------------------
void HomeScreenWidget::onCreateProfile() {
    NewProfileDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.getName();
        if (name.trimmed().isEmpty())
            return;

        Profile profile(name.toStdString(),
                        dlg.getBasalRate(),
                        dlg.getCarbRatio(),
                        dlg.getCorrectionFactor(),
                        dlg.getTargetGlucose());

        m_profileManager->createProfile(profile);
        m_currentProfile = m_profileManager->selectProfile(name.toStdString());

        if (m_basalTimer) {
            m_basalTimer->stop();
            m_basalTimer->deleteLater();
            m_basalTimer = nullptr;
        }
        m_basalPaused = false;
        basalStatusLabel->setText("Basal Delivery not started.");

        // reset all the stuff
        if (m_iob)
            m_iob->updateIOB(0.0f);
        if (m_cartridge)
            m_cartridge->updateInsulinLevel(200.0f);
        if (m_sensor)
            m_sensor->updateGlucoseData(5.5f);

        m_graphWidget->clearGraph();
        updateProfileDisplay();
        updateStatus();
        m_logTextEdit->clear();
        addLog("[PROFILE EVENT] Created profile: " + name);
    }
}

//--------------------------------------------------------
// Profile: Edit current profile
//--------------------------------------------------------
void HomeScreenWidget::onEditProfile() {
    if (!m_currentProfile) {
        QMessageBox::warning(this, "Edit Profile", "No profile loaded.");
        return;
    }

    NewProfileDialog dlg(QString::fromStdString(m_currentProfile->getName()),
                         m_currentProfile->getBasalRate(),
                         m_currentProfile->getCarbRatio(),
                         m_currentProfile->getCorrectionFactor(),
                         m_currentProfile->getTargetGlucose(), this);

    if (dlg.exec() == QDialog::Accepted) {
        m_currentProfile->updateSettings(dlg.getBasalRate(),
                                         dlg.getCarbRatio(),
                                         dlg.getCorrectionFactor(),
                                         dlg.getTargetGlucose());
        updateProfileDisplay();
        addLog("[PROFILE EVENT] Updated profile: " + QString::fromStdString(m_currentProfile->getName()));
    }
}

//--------------------------------------------------------
// Profile: Delete current profile
//--------------------------------------------------------
void HomeScreenWidget::onDeleteProfile() {
    if (!m_currentProfile) {
        QMessageBox::warning(this, "Delete Profile", "No profile loaded.");
        return;
    }

    int ret = QMessageBox::question(this, "Delete Profile",
                                    "Are you sure you want to delete the current profile?",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        addLog("[PROFILE EVENT] Deleted profile: " + QString::fromStdString(m_currentProfile->getName()));
        m_profileManager->deleteProfile(m_currentProfile->getName());
        m_currentProfile = nullptr;
        updateProfileDisplay();
        m_logTextEdit->clear();
    }
}

//--------------------------------------------------------
// Updates profile display label
//--------------------------------------------------------
void HomeScreenWidget::updateProfileDisplay() {
    if (m_currentProfile) {
        currentProfileLabel->setText(
            "Current Profile: \"" + QString::fromStdString(m_currentProfile->getName()) + "\"\n"
            "   - Basal Rate: " + QString::number(m_currentProfile->getBasalRate()) + " u/hr\n"
            "   - Carb Ratio: 1u per " + QString::number(m_currentProfile->getCarbRatio()) + " g\n"
            "   - Correction Factor: 1u per " + QString::number(m_currentProfile->getCorrectionFactor()) + " mmol/L\n"
            "   - Target BG: " + QString::number(m_currentProfile->getTargetGlucose()) + " mmol/L");
    } else {
        currentProfileLabel->setText("No profile loaded.");
    }
}

//--------------------------------------------------------
// Opens bolus calculator dialog and handles result
//--------------------------------------------------------
void HomeScreenWidget::onBolus() {
    if (!m_currentProfile) {
        QMessageBox::warning(this, "Bolus", "No profile loaded.");
        return;
    }

    BolusCalculationDialog dlg(m_currentProfile, m_iob, m_cartridge, m_sensor, this);

    connect(&dlg, &BolusCalculationDialog::mealInfoEntered, this, [this](double newBG) {
        m_sensor->updateGlucoseData(newBG);
        updateStatus();
    });

    connect(&dlg, &BolusCalculationDialog::immediateBolusParameters, this, [this](double bolus) {
        if (m_cartridge && m_cartridge->getInsulinLevel() < bolus) {
            QMessageBox::warning(this, "Insufficient Insulin", "Not enough insulin for this bolus.");
            return;
        }

        if (m_iob)
            m_iob->updateIOB(m_iob->getIOB() + bolus);
        if (m_cartridge)
            m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - bolus);
        if (m_battery)
            m_battery->discharge();

        addLog(QString("[BOLUS EVENT] Immediate Bolus Delivered: %1 u").arg(bolus, 0, 'f', 1));
        updateStatus();
    });

    connect(&dlg, &BolusCalculationDialog::extendedBolusParameters,
        this, [this](double duration, double immediateDose, double extendedDose, double ratePerHour) {
            int totalTicks = static_cast<int>(duration);
            addLog(QString("[BOLUS EVENT] Starting Extended Bolus — Immediate: %1 u, Extended: %2 u over %3 hr")
                       .arg(immediateDose)
                       .arg(extendedDose)
                       .arg(duration));

            QTimer* timer = new QTimer(this);
            int* tick = new int(0);
            connect(timer, &QTimer::timeout, this, [=]() mutable {
                if (*tick < totalTicks) {
                    if (m_iob)
                        m_iob->updateIOB(m_iob->getIOB() + ratePerHour);
                    if (m_cartridge)
                        m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - ratePerHour);

                    updateStatus();
                    addLog(QString("[BOLUS EVENT] Hour %1/%2 | Delivered: %3 u")
                               .arg(*tick + 1)
                               .arg(totalTicks)
                               .arg(ratePerHour, 0, 'f', 2));

                    (*tick)++;
                } else {
                    timer->stop();
                    timer->deleteLater();
                    delete tick;
                    addLog("[BOLUS EVENT] Extended Bolus Completed.");
                }
            });
            timer->start(10000); // 10s per simulated hour
        });

    dlg.exec();
}

//--------------------------------------------------------
// Start basal delivery (checks and logs via BasalManager)
//--------------------------------------------------------

void HomeScreenWidget::startBasalDelivery() {
    if (!m_currentProfile) {
        QMessageBox::warning(this, "Basal Delivery", "No profile loaded.");
        return;
    }
    if (m_battery && m_battery->getStatus() == 0) {
        QMessageBox::warning(this, "Basal Delivery", "Battery depleted. Please charge.");
        return;
    }

    float hourlyRate = m_currentProfile->getBasalRate();
    if (hourlyRate <= 0.0f) {
        QMessageBox::warning(this, "Basal Delivery", "Set a valid basal rate to begin.");
        return;
    }

    m_basalRatePerTick = hourlyRate;
    m_basalPaused = false;

    if (m_basalTimer) {
        m_basalTimer->stop();
        m_basalTimer->deleteLater();
        m_basalTimer = nullptr;
    }

    m_basalTimer = new QTimer(this);
    connect(m_basalTimer, &QTimer::timeout, this, [=]() {
        // NEW CHECK: Don't deliver if simulation is paused
        if (m_simulationPaused || m_basalPaused) return;

        if (m_sensor)
            m_sensor->updateGlucoseData(m_sensor->getGlucoseLevel() - 0.1);

        // new
        if (m_sensor && m_sensor->getGlucoseLevel() < 4.0f) {
            m_basalPaused = true;
            basalStatusLabel->setText("Basal paused: CGM < 3.9 mmol/L");
            addLog("[CONTROL IQ] Basal paused due to low CGM.");
            return;
        }

        if (m_iob)
            m_iob->updateIOB(m_iob->getIOB() + m_basalRatePerTick);
        if (m_cartridge)
            m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - m_basalRatePerTick);
        if (m_battery)
            m_battery->discharge();

        updateStatus();
        addLog(QString("[BASAL EVENT] Delivered: %1 u").arg(m_basalRatePerTick, 0, 'f', 2));
    });

    m_basalTimer->start(10000);  // every 10s
    basalStatusLabel->setText("Basal Delivery in progress...");
    addLog("[BASAL EVENT] Basal Delivery started.");
}


//--------------------------------------------------------
// Simulates charging using a timer
//--------------------------------------------------------
void HomeScreenWidget::onCharge() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    if (m_chargingTimer) {
        m_chargingTimer->stop();
        m_chargingTimer->deleteLater();
        m_chargingTimer = nullptr;
        btn->setStyleSheet("");
        addLog("[SYSTEM] Charging stopped.");
        return;
    }

    btn->setStyleSheet("background-color: green; color: white;");
    addLog("[SYSTEM] Charging started...");

    m_chargingTimer = new QTimer(this);
    connect(m_chargingTimer, &QTimer::timeout, this, [=]() {
        if (m_battery->getStatus() < 100) {
            m_battery->charge();
            updateStatus();
        } else {
            m_chargingTimer->stop();
            m_chargingTimer->deleteLater();
            m_chargingTimer = nullptr;
            btn->setStyleSheet("");
            addLog("[SYSTEM] Charging complete.");
        }
    });
    m_chargingTimer->start(1000); // every second
}

//--------------------------------------------------------
// Scrolls chart and adds new glucose data point
//--------------------------------------------------------
void HomeScreenWidget::updateGraph() {
    if (m_sensor) {
        m_graphWidget->updateGraph(m_sensor->getGlucoseLevel());
    }
}

// repopulate/refresh profile list when options button is clicked

void HomeScreenWidget::refreshProfileList(QComboBox* profileSelector) {
    profileSelector->clear();
    for (const auto& profile : m_profileManager->getProfiles()) {
        QString itemText = QString("%1 - Basal: %2 u/hr, Carb: 1u/%3 g, Correction: 1u/%4 mmol/L, Target: %5 mmol/L")
            .arg(QString::fromStdString(profile.getName()))
            .arg(profile.getBasalRate())
            .arg(profile.getCarbRatio())
            .arg(profile.getCorrectionFactor())
            .arg(profile.getTargetGlucose());
        profileSelector->addItem(itemText, QString::fromStdString(profile.getName()));
    }
}


