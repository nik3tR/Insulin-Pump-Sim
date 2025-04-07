#ifndef BOLUSCALCULATIONDIALOG_H
#define BOLUSCALCULATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

#include "src/models/profile.h"
#include "src/models/iob.h"
#include "src/models/insulincartridge.h"
#include "src/models/cgmsensor.h"
#include "src/logic/bolusmanager.h"

//--------------------------------------------------------
// BOLUS CALCULATION DIALOG (with Extended Bolus Simulation)
//--------------------------------------------------------
class BolusCalculationDialog : public QDialog {
    Q_OBJECT
public:
    explicit BolusCalculationDialog(Profile* profile = nullptr,
                                    IOB* iob = nullptr,
                                    InsulinCartridge* cartridge = nullptr,
                                    CGMSensor* sensor = nullptr,
                                    QWidget* parent = nullptr);

    QLineEdit* getCarbsEdit();

signals:
    void mealInfoEntered(double currentBG);
    void immediateBolusParameters(double immediateDose);
    void extendedBolusParameters(double duration,
                                 double immediateDose,
                                 double extendedDose,
                                 double ratePerHour);

private slots:
    void calculateBolus();

private:
    QStackedWidget*      stackedWidget;
    QLineEdit           *carbsEdit, *bgEdit;
    QLabel              *resultLabel, *formulaLabel;
    QPushButton         *manualButton, *extendedButton;

    Profile*             m_profile;
    double               m_finalBolus;
    IOB*                 m_iob;
    InsulinCartridge*    m_cartridge;
    CGMSensor*           m_sensor;

    BolusManager         m_manager;
};

#endif // BOLUSCALCULATIONDIALOG_H
