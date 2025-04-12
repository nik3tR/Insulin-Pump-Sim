#ifndef PUMPSIMULATORMAINWIDGET_H
#define PUMPSIMULATORMAINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "src/models/insulinpump.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/models/profilemanager.h"
#include "homescreenwidget.h"

//--------------------------------------------------------
// PUMP SIMULATOR MAIN WIDGET (Modified for PIN setup)
//--------------------------------------------------------
class PumpSimulatorMainWidget : public QWidget {
    Q_OBJECT
public:
    PumpSimulatorMainWidget(QWidget* parent = nullptr);
public slots:
    void onPowerToggled(bool checked);
    // Toggle pump power state from Options page.
    void togglePump();
    // Update the stored PIN (used when PIN is changed in options).
    void setUserPIN(const QString &newPIN);
private:
    InsulinPump* m_pump;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    ProfileManager* m_profileManager;
    QPushButton* m_powerButton;
    HomeScreenWidget* m_homeScreen;
    QString m_userPIN;
};

#endif // PUMPSIMULATORMAINWIDGET_H
