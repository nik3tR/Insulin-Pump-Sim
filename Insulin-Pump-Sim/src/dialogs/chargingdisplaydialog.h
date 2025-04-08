#ifndef CHARGINGDISPLAYDIALOG_H
#define CHARGINGDISPLAYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTimer>

class Battery;

//--------------------------------------------------------
// CHARGING DISPLAY DIALOG
//--------------------------------------------------------
class ChargingDisplayDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChargingDisplayDialog(Battery* battery, QWidget* parent = nullptr);

public slots:
    void onTimeout();

private:
    Battery* m_battery;
    QLabel* batteryLabel;
    QTimer* m_timer;
};


#endif // CHARGINGDISPLAYDIALOG_H
