#ifndef HOMESCREENWIDGET_H
#define HOMESCREENWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QStackedWidget>
#include <QtCharts/QChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include "src/models/profilemanager.h"
#include "src/models/battery.h"
#include "src/models/insulincartridge.h"
#include "src/models/iob.h"
#include "src/models/cgmsensor.h"
#include "src/logic/navigationmanager.h"
#include "src/logic/datamanager.h"
#include "optionspagecontroller.h"
#include "src/logic/insulindelivery.h"

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
    void toggleBasalDelivery();
    void startBasalDelivery();
    void updateProfileDisplay();
    void updateHistory();
    void updateGraph();
    void onCrashInsulin();
    void updateOptionsPage();
private:
    QLabel* createStatusBox(const QString& title, const QString& value);
    QLabel *batteryBox, *insulinBox, *iobBox, *cgmBox;
    QLabel *currentProfileLabel;
    QTextEdit* m_logTextEdit;
    QTextEdit* m_historyTextEdit;
    QChart* m_chart;
    QScatterSeries* m_graph_points;
    QScatterSeries* m_predicted_points;
    QSplineSeries* m_graph_line;
    QLabel* basalStatusLabel;
    QStackedWidget* m_mainStackedWidget;
    QVBoxLayout* m_profileButtonsLayout;
    DataManager* m_dataManager;
    ProfileManager* m_profileManager;
    Battery* m_battery;
    InsulinCartridge* m_cartridge;
    IOB* m_iob;
    CGMSensor* m_sensor;
    Profile* m_currentProfile;
    QTimer* m_chargingTimer;
    NavigationManager* m_navManager;
    QPushButton* m_basalButton;
    bool m_alertsEnabled;
    OptionsPageController* m_optionsController;
    InsulinDelivery* m_insulinDelivery;
    void addLog(const QString &message);

};

#endif // HOMESCREENWIDGET_H
