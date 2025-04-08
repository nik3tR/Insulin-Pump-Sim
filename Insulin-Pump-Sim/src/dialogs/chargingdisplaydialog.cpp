#include "chargingdisplaydialog.h"
#include "src/models/battery.h"
#include <QVBoxLayout>
#include <QPushButton>

// Simulates battery charging interface
ChargingDisplayDialog::ChargingDisplayDialog(Battery* battery, QWidget* parent)
    : QDialog(parent), m_battery(battery) {
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

// Simulates charging increments
void ChargingDisplayDialog::onTimeout() {
    if (m_battery->level < 100) {
        m_battery->charge();
        batteryLabel->setText("Battery Level: " + QString::number(m_battery->getStatus()));
    } else {
        m_timer->stop();
    }
}

