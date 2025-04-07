#ifndef BASALMANAGER_H
#define BASALMANAGER_H


#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>

class Profile;
class Battery;
class InsulinCartridge;
class IOB;
class CGMSensor;

//--------------------------------------------------------
// BasalManager: Use Case 6: start, stop, resume Basal Insulin Delivery
//--------------------------------------------------------
class BasalManager : public QObject {
    Q_OBJECT
public:
    BasalManager(Profile* profile, Battery* battery, InsulinCartridge* cartridge,
                 IOB* iob, CGMSensor* sensor, QObject* parent = nullptr);

    void startBasalDelivery(std::function<void(const QString&)> logCallback,
                            std::function<void()> updateStatusCallback,
                            std::function<void(const QString&)> basalStatusCallback);

    void stopBasalDelivery();

private:
    Profile* m_profile;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    QTimer* m_timer = nullptr;
};

#endif // BASALMANAGER_H
