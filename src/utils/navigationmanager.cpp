#include "navigationmanager.h"

#include <QStackedWidget>
#include <iostream>

NavigationManager::NavigationManager(QStackedWidget* stack)
    : m_stack(stack) {}

void NavigationManager::navigateTo(const QString& screen) {
    if (screen == "Home")
        m_stack->setCurrentIndex(0);
    else if (screen == "Options")
        m_stack->setCurrentIndex(1);
    else if (screen == "History")
        m_stack->setCurrentIndex(2);

    std::cout << "[NavigationManager] Navigated to " << screen.toStdString() << "\n";
}

void NavigationManager::navigateToOptions() { navigateTo("Options"); }
void NavigationManager::navigateToHistory() { navigateTo("History"); }
void NavigationManager::navigateToHome()    { navigateTo("Home"); }
void NavigationManager::navigateToBolus()   { navigateTo("Bolus"); }

