#include "pumpsimulatormainwidget.h"
#include "src/dialogs/pindialog.h"
#include <QVBoxLayout>

PumpSimulatorMainWidget::PumpSimulatorMainWidget(QWidget* parent) : QWidget(parent) {
    m_pump = new InsulinPump();
    m_battery = new Battery();
    m_cartridge = new InsulinCartridge();
    m_iob = new IOB();
    m_sensor = new CGMSensor();
    m_profileManager = new ProfileManager();

    // Create the power button and home screen widgets
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

void PumpSimulatorMainWidget::onPowerToggled(bool checked) {
    if (checked) {
        // If no PIN is set, prompt the user to set one (using PinMode::Set)
        if (m_userPIN.isEmpty()) {
            PINDialog setPinDlg(PinMode::Set, "", this);
            if (setPinDlg.exec() == QDialog::Accepted) {
                m_userPIN = setPinDlg.enteredPIN();
            } else {
                m_powerButton->setChecked(false);
                return;
            }
        } else {
            // PIN already set - verify it (using PinMode::Verify)
            PINDialog pinDlg(PinMode::Verify, m_userPIN, this);
            if (pinDlg.exec() != QDialog::Accepted) {
                m_powerButton->setChecked(false);
                return;
            }
        }
        m_pump->powerOn();
        m_powerButton->setText("Power Off");
        m_homeScreen->setVisible(true);
        m_homeScreen->updateStatus();  // Update the home screen indicators
    } else {
        m_pump->powerOff();
        m_powerButton->setText("Power On");
        m_homeScreen->setVisible(false);
    }
}

// NEW: Toggle the pump's power state from the options page.
void PumpSimulatorMainWidget::togglePump() {
    if(m_powerButton->text() == "Power Off") { // pump is currently on
        m_pump->powerOff();
        m_powerButton->setChecked(false);
        m_powerButton->setText("Power On");
        m_homeScreen->setVisible(false);
    } else {
        // Power on the pump: verify PIN if set (or prompt to set)
        if(m_userPIN.isEmpty()) {
            PINDialog setPinDlg(PinMode::Set, "", this);
            if(setPinDlg.exec() == QDialog::Accepted) {
                m_userPIN = setPinDlg.enteredPIN();
            } else {
                return;
            }
        } else {
            PINDialog pinDlg(PinMode::Verify, m_userPIN, this);
            if(pinDlg.exec() != QDialog::Accepted) {
                return;
            }
        }
        m_pump->powerOn();
        m_powerButton->setChecked(true);
        m_powerButton->setText("Power Off");
        m_homeScreen->setVisible(true);
        m_homeScreen->updateStatus();
    }
}

// NEW: Setter to update the stored user PIN.
void PumpSimulatorMainWidget::setUserPIN(const QString &newPIN) {
    m_userPIN = newPIN;
}
