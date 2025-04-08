#include "datamanager.h"

#include <QDateTime>

void DataManager::logEvent(const QString& event) {
    QString timestamped = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + event;
    eventHistory.append(timestamped);
}

QString DataManager::getHistory() const {
    return eventHistory.join("\n");
}

QString DataManager::analyzeUsage() const {
    return "Usage analysis not implemented.";
}
