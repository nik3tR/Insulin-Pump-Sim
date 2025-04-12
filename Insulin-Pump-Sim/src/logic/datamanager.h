#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QString>
#include <QStringList>
#include <QDateTime>

//--------------------------------------------------------
// DATA MANAGER (New for event history logging)
//--------------------------------------------------------
class DataManager {
public:
    void logEvent(const QString& event);
    QString getHistory() const;
    // Placeholder for potential future usage analysis.
    QString analyzeUsage() const;
private:
    QStringList eventHistory;
};

#endif // DATAMANAGER_H
