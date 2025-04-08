#include "insulinpump.h"
#include <iostream>

InsulinPump::InsulinPump()
    : state(PumpState::Off), currentProfile(nullptr) {}

void InsulinPump::powerOn() {
    state = PumpState::On;
}

void InsulinPump::powerOff() {
    state = PumpState::Off;
}

void InsulinPump::navigateTo(const std::string& screen) {
    std::cout << "Navigating to " << screen << "\n";
}

void InsulinPump::updateDisplay() {
    std::cout << "Display updated.\n";
}
