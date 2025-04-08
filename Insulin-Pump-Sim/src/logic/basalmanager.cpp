#include "basalmanager.h"
#include "src/models/profile.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/logic/controliq.h"

#include <QString>

// Constructor
BasalManager::BasalManager(Profile* profile, Battery* battery, InsulinCartridge* cartridge,
                           IOB* iob, CGMSensor* sensor, QObject* parent)
    : QObject(parent), m_profile(profile), m_battery(battery), m_cartridge(cartridge),
      m_iob(iob), m_sensor(sensor) {}

// Starts basal insulin delivery. Handles alerts, battery, occlusion, CGM data, etc.
void BasalManager::startBasalDelivery(std::function<void(const QString&)> logCallback,
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
        // Battery check
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

        // CGM disconnected
        if (m_sensor && !m_sensor->isConnected()) {
            logCallback("[SYSTEM EVENT] CGM sensor disconnected. Basal delivery suspended.");
            basalStatusCallback("Basal Delivery Suspended (CGM Disconnected)");
            m_timer->stop();
            m_timer->deleteLater();
            return;
        }

        // Occlusion detected
        if (m_cartridge && m_cartridge->isOccluded()) {
            logCallback("[SYSTEM EVENT] Occlusion detected. Check infusion site. Basal delivery suspended.");
            basalStatusCallback("Basal Delivery Suspended (Occlusion Detected)");
            m_timer->stop();
            m_timer->deleteLater();
            return;
        }

        float cgm = m_sensor->getGlucoseLevel();
        double adjustment = controlIQ.adjustDelivery(cgm);
        float adjustedRate = rate * adjustment;

        if (cgm < 4.0f) {
            basalStatusCallback("Basal Paused (Low CGM)");
            logCallback("[BASAL EVENT] Basal Delivery Paused — CGM too low (< 3.9 mmol/L)");
            return;
        }

        // Deliver insulin + update IOB
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

    m_timer->start(10000);  // Deliver insulin every 10 seconds
}

// Stop basal delivery
void BasalManager::stopBasalDelivery() {
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
}

