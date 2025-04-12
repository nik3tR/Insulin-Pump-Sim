#ifndef HOMESCREENWIDGET_H
#define HOMESCREENWIDGET_H

#include <QWidget>
#include "src/models/profilemanager.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/models/profile.h"
#include "src/dialogs/newprofiledialog.h"
#include "src/dialogs/boluscalculationdialog.h"
#include "src/dialogs/chargingdisplaydialog.h"
#include "src/utils/navigationmanager.h"
#include "src/utils/datamanager.h"
#include "src/logic/basalmanager.h"
#include "src/widgets/graphwidget.h"

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QTextEdit>
#include <QMessageBox>
#include <QFrame>
#include <QPainter>
#include <QDateTime>
#include <QComboBox>

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
    void refreshProfileList(QComboBox* profileSelector);

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
