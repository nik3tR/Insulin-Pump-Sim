#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QString>
#include <QStringList>

//--------------------------------------------------------
// DATA MANAGER
// Responsible for logging time-stamped events and exposing history
//--------------------------------------------------------
class DataManager {
public:
    void logEvent(const QString& event);
    QString getHistory() const;
    QString analyzeUsage() const; // placeholder for future insights

private:
    QStringList eventHistory;
};

#endif // DATAMANAGER_H
