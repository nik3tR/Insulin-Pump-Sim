#include "OptionsPageController.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>

OptionsPageController::OptionsPageController(QWidget* parent, ProfileManager* profileMgr)
    : QObject(parent), m_profileMgr(profileMgr)
{
    m_optionsPage = new QWidget(parent);
    m_optionsLayout = new QVBoxLayout(m_optionsPage);
    m_optionsLayout->addWidget(new QLabel("Options Screen", m_optionsPage));

    // Box for profile switching
    QGroupBox* switchProfileGroup = new QGroupBox("Switch Profile", m_optionsPage);
    m_profileButtonsLayout = new QVBoxLayout();
    switchProfileGroup->setLayout(m_profileButtonsLayout);
    m_optionsLayout->addWidget(switchProfileGroup);

    // Alert toggle -> disable alrts when they click on the box
    m_alertToggle = new QCheckBox("Disable Alerts", m_optionsPage);
    m_alertToggle->setChecked(false);
    m_optionsLayout->addWidget(m_alertToggle);
    connect(m_alertToggle, &QCheckBox::toggled, this, &OptionsPageController::alertToggled);

    // Set / Change PIN button
    m_changePinButton = new QPushButton("Set / Change PIN", m_optionsPage);
    m_optionsLayout->addWidget(m_changePinButton);
    connect(m_changePinButton, &QPushButton::clicked, this, &OptionsPageController::changePinRequested);

    // sleep timeout
    m_sleepTimeoutBox = new QSpinBox(m_optionsPage);
    m_sleepTimeoutBox->setRange(5, 120);
    m_sleepTimeoutBox->setValue(30);
    QHBoxLayout* timeoutLayout = new QHBoxLayout();
    timeoutLayout->addWidget(new QLabel("Sleep Timeout (s):", m_optionsPage));
    timeoutLayout->addWidget(m_sleepTimeoutBox);
    m_optionsLayout->addLayout(timeoutLayout);

    // toglging Sleep Mode
    m_sleepModeToggle = new QCheckBox("Enable Sleep Mode", m_optionsPage);
    m_sleepModeToggle->setChecked(false);
    m_optionsLayout->addWidget(m_sleepModeToggle);
    connect(m_sleepModeToggle, &QCheckBox::toggled, this, [this](bool enabled){
        int timeout = m_sleepTimeoutBox->value();
        emit sleepModeToggled(enabled, timeout);
    });

    // Power Off Pump button
    m_powerOffButton = new QPushButton("Power Off Pump", m_optionsPage);
    m_powerOffButton->setStyleSheet("background-color: red; color: white;");
    m_optionsLayout->addWidget(m_powerOffButton);
    connect(m_powerOffButton, &QPushButton::clicked, this, &OptionsPageController::powerOffRequested);

    // button
    m_togglePumpButton = new QPushButton("Toggle Pump", m_optionsPage);
    m_optionsLayout->addWidget(m_togglePumpButton);
    connect(m_togglePumpButton, &QPushButton::clicked, this, &OptionsPageController::togglePumpRequested);

    // Back button
    m_backButton = new QPushButton("Back", m_optionsPage);
    m_optionsLayout->addWidget(m_backButton);
    connect(m_backButton, &QPushButton::clicked, this, &OptionsPageController::backClicked);
}

QWidget* OptionsPageController::getWidget() const {
    return m_optionsPage;
}

void OptionsPageController::updateProfileSwitching(const std::vector<Profile>& profiles,
                                                   Profile* currentProfile,
                                                   std::function<void(const Profile&)> onProfileSwitch)
{
    //Get rid of the buttons for the profile
    QLayoutItem* child;
    while ((child = m_profileButtonsLayout->takeAt(0)) != nullptr) {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    bool anyAdded = false;
    for(const auto& profile : profiles) {
        if(currentProfile && profile.getName() == currentProfile->getName())
            continue;
        QPushButton* profileButton = new QPushButton(QString::fromStdString(profile.getName()), m_optionsPage);
        connect(profileButton, &QPushButton::clicked, [onProfileSwitch, profile]() {
            onProfileSwitch(profile);
        });
        m_profileButtonsLayout->addWidget(profileButton);
        anyAdded = true;
    }
    if (!anyAdded) {
        QLabel* noProfileLabel = new QLabel("No other profiles available", m_optionsPage);
        m_profileButtonsLayout->addWidget(noProfileLabel);
    }
}
