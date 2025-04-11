#ifndef HOMESCREENWIDGET_H
#define HOMESCREENWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QLineSeries>
#include "graphwidget.h"

QT_CHARTS_USE_NAMESPACE

class ProfileManager;
class Battery;
class InsulinCartridge;
class IOB;
class CGMSensor;
class QLabel;
class QTextEdit;
class QTimer;
class QStackedWidget;
class Profile;
class DataManager;
class NavigationManager;

//--------------------------------------------------------
// HOME SCREEN WIDGET (Pump UI and Simulation Controls)
//--------------------------------------------------------
class HomeScreenWidget : public QWidget {
    Q_OBJECT
public:
    HomeScreenWidget(ProfileManager* profileManager,
                     Battery* battery,
                     InsulinCartridge* cartridge,
                     IOB* iob,
                     CGMSensor* sensor,
                     QWidget* parent = nullptr);

public slots:
    void updateStatus();
    void onCreateProfile();
    void onEditProfile();
    void onDeleteProfile();
    void onBolus();
    void onCharge();
    void startBasalDelivery();

private:
    QLabel* createStatusBox(const QString& title, const QString& value);
    void updateProfileDisplay();
    void addLog(const QString& message);
    void updateHistory();
    void updateGraph();

    QLabel *batteryBox, *insulinBox, *iobBox, *cgmBox;
    QLabel *currentProfileLabel;
    QTextEdit *m_logTextEdit;
    QTextEdit *m_historyTextEdit;
    QLabel *basalStatusLabel;
    QStackedWidget* m_mainStackedWidget;

    bool m_simulationPaused = false; // new
    QTimer* m_basalTimer = nullptr;
    float m_basalRatePerTick = 0.0f; // basically is the basal rate per hour in the simulation, tick is anotherway to reefer to this time stamp
    bool m_basalPaused = false;

    ProfileManager* m_profileManager;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    Profile* m_currentProfile;
    QTimer* m_chargingTimer;
    GraphWidget* m_graphWidget;

    DataManager* m_dataManager;
    NavigationManager* m_navManager;
};

#endif // HOMESCREENWIDGET_H
