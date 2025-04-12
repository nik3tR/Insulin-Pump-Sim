#ifndef INSULINDELIVERY_H
#define INSULINDELIVERY_H

#include <QObject>
#include <functional>
#include "src/models/profile.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "basalmanager.h"
#include "bolusmanager.h"

class QWidget;

class InsulinDelivery : public QObject {
    Q_OBJECT
public:
    // Constructor
    InsulinDelivery(Profile*& currentProfile,
                              Battery* battery,
                              InsulinCartridge* cartridge,
                              IOB* iob,
                              CGMSensor* sensor,
                              std::function<void(const QString&)> addLogCallback,
                              std::function<void()> updateStatusCallback,
                              std::function<void(const QString&)> updateBasalStatusCallback,
                              QObject* parent = nullptr);
    //bolus calculation
    void launchBolusDialog(QWidget* parentWidget);
    //  (start/pause/resume)
    void toggleBasalDelivery();
    // Starts basal delyver
    void startBasalDelivery();
    // Update the profile.
    void setCurrentProfile(Profile* profile);
    void stopAllDelivery();


private:
    Profile*& m_currentProfile;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    std::function<void(const QString&)> m_addLog;
    std::function<void()> m_updateStatus;
    std::function<void(const QString&)> m_updateBasalStatus;
    BasalManager* m_basalManager;
    bool m_basalRunning;
    bool m_basalPaused;
};

#endif // INSULINDELIVERYCONTROLLER_H
