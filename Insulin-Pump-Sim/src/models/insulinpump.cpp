#include "insulinpump.h"

InsulinPump::InsulinPump() : state(Off), currentProfile(nullptr) {}

void InsulinPump::powerOn() {
    state = On;
}

void InsulinPump::powerOff() {
    state = Off;
}

void InsulinPump::navigateTo(const std::string& screen) {
    std::cout << "Navigating to " << screen << "\n";
}

void InsulinPump::updateDisplay() {
    std::cout << "Display updated.\n";
}
