#include "basalmanager.h"

BasalManager::BasalManager(Profile* profile, Battery* battery, InsulinCartridge* cartridge, IOB* iob, CGMSensor* sensor, QObject* parent)
    : QObject(parent),
    m_profile(profile),
    m_battery(battery),
    m_cartridge(cartridge),
    m_iob(iob),
    m_sensor(sensor),
    m_timer(nullptr),
    m_isPaused(false)
{}

void BasalManager::startBasalDelivery(std::function<void(const QString&)> logCallback,
                                      std::function<void()> updateStatusCallback,
                                      std::function<void(const QString&)> basalStatusCallback)
{
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
        logCallback("[BASAL] Set a valid basal rate in the profile to start delivery.");
        return;
    }

    logCallback(QString("[BASAL] Basal Delivery started at %1 u/hr").arg(rate));
    m_timer = new QTimer(this);
    ControlIQ controlIQ;

    connect(m_timer, &QTimer::timeout, this, [=]() mutable {
        // Battery Check

        if (m_battery && m_battery->getStatus() <= 20) {
            logCallback("[SYSTEM] 🪫 Low Battery ->  Battery is low -> Deliverying final doses.");
        }


        if (m_battery && m_battery->getStatus() == 0) {
            logCallback("Battery fully drained -> Basal Delivery paused.");
            basalStatusCallback("Basal Paused (Battery 0%)");
            pause();
            return;
        }


        // CGM Disconnection / Occlusion
        if (m_sensor && !m_sensor->isConnected()) {
            logCallback("[SYSTEM] 🚫 CGM disconnected. Basal delivery paused.");
            basalStatusCallback("Basal Paused (CGM Disconnected)");
            pause();
            return;
        }
        if (m_cartridge && m_cartridge->isOccluded()) {
            logCallback("[SYSTEM] ❌ Occlusion detected. Basal delivery paused.");
            basalStatusCallback("Basal Paused (Occlusion)");
            pause();
            return;
        }

        float cgm = m_sensor->getGlucoseLevel();
        double adjustment = controlIQ.adjustDelivery(cgm);
        float adjustedRate = rate * adjustment;

        if (cgm < 4.0f) {
            basalStatusCallback("Basal Paused (Low CGM)");
            logCallback("[BASAL] Basal Delivery Paused — CGM too low (< 4.0 mmol/L)");
            pause();
            return;
        }

        // Insulin Delivery Logic
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
        logCallback(QString("[BASAL] Basal Delivered: %1 u | CGM: %2 mmol/L")
                        .arg(adjustedRate, 0, 'f', 1)
                        .arg(m_sensor->getGlucoseLevel(), 0, 'f', 1));
    });

    m_timer->start(10000);
    m_isPaused = false;
}

void BasalManager::pause() {
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
        m_isPaused = true;
    }
}

void BasalManager::resume() {
    if (m_timer && m_isPaused) {
        m_timer->start(10000);
        m_isPaused = false;
    }
}

void BasalManager::stop() {
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    m_isPaused = false;
}

bool BasalManager::isPaused() const {
    return m_isPaused;
}
