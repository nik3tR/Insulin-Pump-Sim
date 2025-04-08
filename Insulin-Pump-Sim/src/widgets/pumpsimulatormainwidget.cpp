#include "pumpsimulatormainwidget.h"

#include "src/models/insulinpump.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/models/profilemanager.h"

#include "src/widgets/homescreenwidget.h"
#include "src/dialogs/setpindialog.h"
#include "src/dialogs/pindialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

PumpSimulatorMainWidget::PumpSimulatorMainWidget(QWidget* parent)
    : QWidget(parent)
{
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

void PumpSimulatorMainWidget::onPowerToggled(bool checked) {
    if (checked) {
        // Prompt for PIN if it's already set
        if (!m_userPIN.isEmpty()) {
            PINDialog pinDlg(m_userPIN, this);
            if (pinDlg.exec() != QDialog::Accepted) {
                m_powerButton->setChecked(false);
                return;
            }
        } else {
            // First time setup: Set PIN
            SetPinDialog setPinDlg(this);
            if (setPinDlg.exec() == QDialog::Accepted) {
                m_userPIN = setPinDlg.getPin();
            } else {
                m_powerButton->setChecked(false);
                return;
            }
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
