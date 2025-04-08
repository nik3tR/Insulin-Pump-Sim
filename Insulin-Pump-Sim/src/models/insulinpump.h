#ifndef INSULINPUMP_H
#define INSULINPUMP_H


#include "src/enums/PumpState.h"
#include <string>

class Profile;

class InsulinPump {
public:
    PumpState state;
    Profile* currentProfile;

    InsulinPump();

    void powerOn();
    void powerOff();
    void navigateTo(const std::string& screen);
    void updateDisplay();
};

#endif // INSULINPUMP_H
