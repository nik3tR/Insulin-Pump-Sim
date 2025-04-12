#ifndef EXTENDEDBOLUSDIALOG_H
#define EXTENDEDBOLUSDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>

//--------------------------------------------------------
// EXTENDED BOLUS DIALOG
//--------------------------------------------------------
class ExtendedBolusDialog : public QDialog {
    Q_OBJECT
public:
    ExtendedBolusDialog(QWidget* parent = nullptr);
    double getDuration() const;
    double getImmediatePercentage() const;
    double getExtendedPercentage() const;
private:
    QLineEdit *durationEdit;
    QLineEdit *immediateEdit;
    QLineEdit *extendedEdit;
};

#endif // EXTENDEDBOLUSDIALOG_H
