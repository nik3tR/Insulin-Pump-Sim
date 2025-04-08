#include "battery.h"

Battery::Battery() : level(100) {}

void Battery::discharge() {
    if (level > 0) level -= 10;
    if (level < 0) level = 0;
}

int Battery::getStatus() const {
    return level;
}

void Battery::charge() {
    if (level < 100) level++;
}
