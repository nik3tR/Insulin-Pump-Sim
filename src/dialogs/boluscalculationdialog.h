#ifndef BOLUSCALCULATIONDIALOG_H
#define BOLUSCALCULATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

class Profile;
class IOB;
class InsulinCartridge;
class CGMSensor;

//--------------------------------------------------------
// BOLUS CALCULATION DIALOG (with Extended Bolus Simulation)
//--------------------------------------------------------
class BolusCalculationDialog : public QDialog {
    Q_OBJECT
public:
    explicit BolusCalculationDialog(Profile* profile = nullptr, IOB* iob = nullptr,
                                    InsulinCartridge* cartridge = nullptr, CGMSensor* sensor = nullptr,
                                    QWidget* parent = nullptr);

    QLineEdit* getCarbsEdit();

signals:
    void extendedBolusParameters(double duration, double immediateDose, double extendedDose, double ratePerHour);
    void immediateBolusParameters(double immediateDose);
    void mealInfoEntered(double currentBG);

private slots:
    void calculateBolus();

private:
    QStackedWidget* stackedWidget;
    QLineEdit *carbsEdit, *bgEdit;
    QLabel *resultLabel, *formulaLabel;
    QPushButton *manualButton, *extendedButton;

    Profile* m_profile;
    double m_finalBolus;
    IOB* m_iob;
    InsulinCartridge* m_cartridge;
    CGMSensor* m_sensor;
};


#endif // BOLUSCALCULATIONDIALOG_H
