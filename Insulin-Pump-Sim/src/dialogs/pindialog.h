#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QDialog>
#include <QLineEdit>

//--------------------------------------------------------
// PIN DIALOG
//--------------------------------------------------------
class PINDialog : public QDialog {
    Q_OBJECT
public:
    explicit PINDialog(const QString& expectedPin, QWidget* parent = nullptr);
    QString enteredPIN() const;

private slots:
    void verifyPIN();

private:
    QLineEdit* pinEdit;
    QString m_expectedPin;
};


#endif // PINDIALOG_H
