#ifndef SETPINDIALOG_H
#define SETPINDIALOG_H

#include <QDialog>
#include <QLineEdit>

class SetPinDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetPinDialog(QWidget* parent = nullptr);
    QString getPin() const;

private slots:
    void verifyPin();

private:
    QLineEdit* pinEdit;
};


#endif // SETPINDIALOG_H
