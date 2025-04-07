#ifndef NAVIGATIONMANAGER_H
#define NAVIGATIONMANAGER_H

#include <QString>

class QStackedWidget;

//--------------------------------------------------------
// NAVIGATION MANAGER (Navigation)
//--------------------------------------------------------
class NavigationManager {
public:
    NavigationManager(QStackedWidget* stack);

    void navigateTo(const QString& screen);
    void navigateToOptions();
    void navigateToHistory();
    void navigateToHome();
    void navigateToBolus();

private:
    QStackedWidget* m_stack;
};

#endif // NAVIGATIONMANAGER_H
