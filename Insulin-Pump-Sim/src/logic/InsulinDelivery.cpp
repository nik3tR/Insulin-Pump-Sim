#include "insulindelivery.h"
#include "src/dialogs/boluscalculationdialog.h"
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

InsulinDelivery::InsulinDelivery(Profile*& currentProfile,
                                                     Battery* battery,
                                                     InsulinCartridge* cartridge,
                                                     IOB* iob,
                                                     CGMSensor* sensor,
                                                     std::function<void(const QString&)> addLogCallback,
                                                     std::function<void()> updateStatusCallback,
                                                     std::function<void(const QString&)> updateBasalStatusCallback,
                                                     QObject* parent)
    : QObject(parent),
    m_currentProfile(currentProfile),
    m_battery(battery),
    m_cartridge(cartridge),
    m_iob(iob),
    m_sensor(sensor),
    m_addLog(addLogCallback),
    m_updateStatus(updateStatusCallback),
    m_updateBasalStatus(updateBasalStatusCallback),
    m_basalManager(nullptr),
    m_basalRunning(false),
    m_basalPaused(false)
{
}

void InsulinDelivery::launchBolusDialog(QWidget* parentWidget) {
    BolusCalculationDialog dlg(m_currentProfile, m_iob, m_cartridge, m_sensor, parentWidget);
    connect(&dlg, &BolusCalculationDialog::mealInfoEntered, parentWidget, [=](double newBG) {
        m_sensor->updateGlucoseData(newBG);
        m_updateStatus();
        QTimer* mealRiseTimer = new QTimer(parentWidget);
        connect(mealRiseTimer, &QTimer::timeout, parentWidget, [=]() {
            float currentBG = m_sensor->getGlucoseLevel();
            float targetMealBG = newBG + 2.0f;
            if (currentBG < targetMealBG) {
                m_sensor->updateGlucoseData(currentBG + 0.5f);
                m_updateStatus();
                m_addLog(QString("🍔 Meal Eaten: CGM increased to %1 mmol/L").arg(m_sensor->getGlucoseLevel(), 0, 'f', 1));
            } else {
                mealRiseTimer->stop();
                mealRiseTimer->deleteLater();
                m_addLog("📈 CGM rise finished.");
            }
        });
        mealRiseTimer->start(5000);
    });
    connect(&dlg, &BolusCalculationDialog::extendedBolusParameters, parentWidget, [=](double duration, double immediateDose, double extendedDose, double ratePerHour) {
        int totalTicks = static_cast<int>(duration);
        m_addLog(QString("[BOLUS] Starting Extended Bolus Delivery... Immediate: %1 u, Extended: %2 u over %3 hrs at %4 u/hr")
                     .arg(immediateDose)
                     .arg(extendedDose)
                     .arg(totalTicks)
                     .arg(ratePerHour, 0, 'f', 2));
        QTimer* timer = new QTimer(parentWidget);
        int* tick = new int(0);
        connect(timer, &QTimer::timeout, parentWidget, [=]() mutable {
            if (*tick < totalTicks) {
                if (m_iob)
                    m_iob->updateIOB(m_iob->getIOB() + ratePerHour);
                if (m_cartridge)
                    m_cartridge->updateInsulinLevel(m_cartridge->getInsulinLevel() - ratePerHour);
                m_updateStatus();
                m_addLog(QString("[BOLUS] %1/%2 hrs | +%3 u delivered (extended)")
                             .arg(*tick + 1)
                             .arg(totalTicks)
                             .arg(ratePerHour, 0, 'f', 2));
                (*tick)++;
            } else {
                timer->stop();
                timer->deleteLater();
                delete tick;
                m_addLog("[BOLUS] ✅ Extended Bolus Completed");
            }
        });
        timer->start(10000);
        QTimer* cgmTimer = new QTimer(parentWidget);
        connect(cgmTimer, &QTimer::timeout, parentWidget, [=]() {
            double currentBG = m_sensor->getGlucoseLevel();
            double targetBG = (m_currentProfile) ? m_currentProfile->getTargetGlucose() : 5.0;
            if (currentBG > targetBG) {
                double updated = currentBG - 0.5;
                if (updated < targetBG)
                    updated = targetBG;
                m_sensor->updateGlucoseData(updated);
                m_updateStatus();
                m_addLog(QString("[BOLUS] CGM: %1 mmol/L").arg(updated, 0, 'f', 2));
            } else {
                cgmTimer->stop();
                cgmTimer->deleteLater();
                m_addLog("[BOLUS] ✅ CGM @ Target: Complete");
            }
        });
        cgmTimer->start(10000);
    });
    connect(&dlg, &BolusCalculationDialog::immediateBolusParameters, parentWidget, [=](double bolus) {
        if (m_cartridge && m_cartridge->getInsulinLevel() < bolus) {
            QMessageBox::warning(parentWidget, "Insufficient Insulin",
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
        m_addLog(QString("[BOLUS] Immediate Bolus Delivered: %1 u").arg(bolus, 0, 'f', 1));
        m_updateStatus();
        QTimer* cgmTimer = new QTimer(parentWidget);
        connect(cgmTimer, &QTimer::timeout, parentWidget, [=]() {
            double currentBG = m_sensor->getGlucoseLevel();
            double targetBG = (m_currentProfile) ? m_currentProfile->getTargetGlucose() : 5.0;
            if (currentBG > targetBG) {
                double updated = currentBG - 0.5;
                if (updated < targetBG)
                    updated = targetBG;
                m_sensor->updateGlucoseData(updated);
                m_updateStatus();
                m_addLog(QString("[BOLUS] CGM updated: %1 mmol/L").arg(updated, 0, 'f', 2));
            } else {
                cgmTimer->stop();
                cgmTimer->deleteLater();
                m_addLog("[BOLUS] ✅ CGM simulation complete.");
            }
        });
        cgmTimer->start(10000);
    });
    dlg.exec();
}

void InsulinDelivery::toggleBasalDelivery() {
    if (!m_currentProfile) {
        QMessageBox::warning(nullptr, "Basal Delivery", "No profile loaded.");
        return;
    }
    if (m_basalManager == nullptr) {
        m_basalManager = new BasalManager(m_currentProfile, m_battery, m_cartridge, m_iob, m_sensor, this);
        m_basalManager->startBasalDelivery(
            [this](const QString& msg){ m_addLog(msg); },
            [this](){ m_updateStatus(); },
            [this](const QString& status){ m_updateBasalStatus(status); }
            );
        m_basalRunning = true;
        m_basalPaused = false;
        m_addLog("[BASAL] Basal delivery started.");
    } else if (m_basalRunning && !m_basalPaused) {
        m_basalManager->pause();
        m_basalPaused = true;
        m_addLog("[BASAL] Basal delivery paused.");
    } else if (m_basalRunning && m_basalPaused) {
        m_basalManager->resume();
        m_basalPaused = false;
        m_addLog("[BASAL] Basal delivery resumed.");
    }
}

void InsulinDelivery::startBasalDelivery() {
    if (!m_currentProfile) {
        QMessageBox::warning(nullptr, "Basal Delivery", "No profile loaded.");
        return;
    }
    if (m_battery && m_battery->getStatus() == 0) {
        QMessageBox::warning(nullptr, "Basal Delivery", "🪫 NO BATTERY ->  Please charge the pump.");
        return;
    }
    float rate = m_currentProfile->getBasalRate();
    if (rate <= 0.0f) {
        QMessageBox::warning(nullptr, "Basal Delivery", "Set a valid basal rate in the profile to start delivery.");
        return;
    }
    BasalManager* basalMgr = new BasalManager(m_currentProfile, m_battery, m_cartridge, m_iob, m_sensor, this);
    basalMgr->startBasalDelivery(
        [this](const QString &msg){ m_addLog(msg); },
        [this](){ m_updateStatus(); },
        [this](const QString &status){ m_updateBasalStatus(status); }
        );
}

void InsulinDelivery::setCurrentProfile(Profile* profile) {
    m_currentProfile = profile;
}

void InsulinDelivery::stopAllDelivery() {
    if (m_basalManager) {
        m_basalManager->stop();
        m_basalRunning = false;
        m_basalPaused = false;
        m_updateBasalStatus("Basal stopped (System Crash)");
        m_addLog("[BASAL] ⛔ All basal delivery stopped.");
    }
}

