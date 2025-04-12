#ifndef INSULINPUMP_H
#define INSULINPUMP_H

#include <iostream>
#include <string>

//--------------------------------------------------------
// ENUMS
//--------------------------------------------------------
enum PumpState {
    Off,
    On,
    Charging,
    Error
};

class Profile;

//--------------------------------------------------------
// INSULIN PUMP
//--------------------------------------------------------
class InsulinPump {
public:
    PumpState state;
    Profile* currentProfile;
    InsulinPump();
    
    // Power on/off the pump (Use Case 1)
    void powerOn();
    void powerOff();
    void navigateTo(const std::string& screen);
    void updateDisplay();
};

#endif // INSULINPUMP_H
