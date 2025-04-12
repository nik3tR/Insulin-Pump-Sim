#ifndef OPTIONSPAGECONTROLLER_H
#define OPTIONSPAGECONTROLLER_H

#include <QObject>
#include <vector>
#include <functional>
#include "src/models/profile.h"
#include "src/models/profilemanager.h"

class QWidget;
class QVBoxLayout;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QLabel;

class OptionsPageController : public QObject {
    Q_OBJECT
public:
    explicit OptionsPageController(QWidget* parent, ProfileManager* profileMgr);
    QWidget* getWidget() const;


    // Update the profile-switching
    void updateProfileSwitching(const std::vector<Profile>& profiles,
                                Profile* currentProfile,
                                std::function<void(const Profile&)> onProfileSwitch);
signals:
    void alertToggled(bool disabled);
    void changePinRequested();
    void sleepModeToggled(bool enabled, int timeout);
    void powerOffRequested();
    void togglePumpRequested();
    void backClicked();

private:
    QWidget* m_optionsPage;
    QVBoxLayout* m_optionsLayout;
    QVBoxLayout* m_profileButtonsLayout;
    QCheckBox* m_alertToggle;
    QSpinBox* m_sleepTimeoutBox;
    QCheckBox* m_sleepModeToggle;
    QPushButton* m_changePinButton;
    QPushButton* m_powerOffButton;
    QPushButton* m_togglePumpButton;
    QPushButton* m_backButton;

    ProfileManager* m_profileMgr;
};

#endif // OPTIONSPAGECONTROLLER_H
