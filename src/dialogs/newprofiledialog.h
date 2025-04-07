#ifndef NEWPROFILEDIALOG_H
#define NEWPROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>

//--------------------------------------------------------
// NEW / EDIT PROFILE DIALOG
//--------------------------------------------------------
class NewProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewProfileDialog(QWidget* parent = nullptr);
    NewProfileDialog(const QString& name, float basal, float carb, float correction, float target, QWidget* parent = nullptr);

    QString getName() const;
    float getBasalRate() const;
    float getCarbRatio() const;
    float getCorrectionFactor() const;
    float getTargetGlucose() const;

private:
    QLineEdit *nameEdit, *basalEdit, *carbEdit, *correctionEdit, *targetEdit;
    void initializeUI(const QString& title);
};


#endif // NEWPROFILEDIALOG_H
