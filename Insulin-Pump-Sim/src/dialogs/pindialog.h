#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QDialog>
#include <QLineEdit>

// Enum to specify dialog mode
enum class PinMode {
    Verify,
    Set
};

class PINDialog : public QDialog {
    Q_OBJECT
public:
    explicit PINDialog(PinMode mode, const QString& expectedPin = "", QWidget* parent = nullptr);
    QString enteredPIN() const;

private slots:
    void verifyPIN();

private:
    QLineEdit* pinEdit;
    QString m_expectedPin;
    PinMode m_mode;
};

#endif // PINDIALOG_H
