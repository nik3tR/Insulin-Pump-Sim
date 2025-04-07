#ifndef PUMPSIMULATORMAINWIDGET_H
#define PUMPSIMULATORMAINWIDGET_H

#include <QWidget>
#include <QString>

class InsulinPump;
class Battery;
class InsulinCartridge;
class IOB;
class CGMSensor;
class ProfileManager;
class QPushButton;
class HomeScreenWidget;

//--------------------------------------------------------
// PUMP SIMULATOR MAIN WIDGET
// This class acts as the entry widget for the simulator.
//--------------------------------------------------------
class PumpSimulatorMainWidget : public QWidget {
    Q_OBJECT
public:
    explicit PumpSimulatorMainWidget(QWidget* parent = nullptr);

private slots:
    void onPowerToggled(bool checked);

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
