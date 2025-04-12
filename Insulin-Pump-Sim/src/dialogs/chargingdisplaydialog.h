#ifndef CHARGINGDISPLAYDIALOG_H
#define CHARGINGDISPLAYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "src/models/battery.h"

//--------------------------------------------------------
// CHARGING DISPLAY DIALOG (unchanged)
//--------------------------------------------------------
class ChargingDisplayDialog : public QDialog {
    Q_OBJECT
public:
    ChargingDisplayDialog(Battery* battery, QWidget* parent = nullptr);
public slots:
    // Increases battery level until fully charged.
    void onTimeout();
private:
    Battery* m_battery;
    QLabel* batteryLabel;
    QTimer* m_timer;
};

#endif // CHARGINGDISPLAYDIALOG_H
