#ifndef BASALMANAGER_H
#define BASALMANAGER_H

#include <QObject>
#include <QTimer>
#include <functional>
#include "src/models/profile.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/logic/controliq.h"

class BasalManager : public QObject {
    Q_OBJECT

public:
    BasalManager(Profile* profile,
                 Battery* battery,
                 InsulinCartridge* cartridge,
                 IOB* iob,
                 CGMSensor* sensor,
                 QObject* parent = nullptr);

    void startBasalDelivery(std::function<void(const QString&)> logCallback,
                            std::function<void()> updateStatusCallback,
                            std::function<void(const QString&)> basalStatusCallback);

    void pause();
    void resume();
    void stop();  // clean stop and reset
    bool isPaused() const;

private:
    Profile* m_profile;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    QTimer* m_timer;
    bool m_isPaused;
};

#endif // BASALMANAGER_H
