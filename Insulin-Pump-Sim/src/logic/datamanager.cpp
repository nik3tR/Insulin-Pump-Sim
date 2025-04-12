#include "datamanager.h"

void DataManager::logEvent(const QString& event) {
    QString timeStamped = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + event;
    eventHistory.append(timeStamped);
}

QString DataManager::getHistory() const {
    return eventHistory.join("\n");
}

QString DataManager::analyzeUsage() const {
    return "Usage analysis not implemented.";
}
