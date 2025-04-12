#include "iob.h"

IOB::IOB() : activeInsulinUnits(0.0f) {}

void IOB::updateIOB(float units) {
    activeInsulinUnits = units;
}

float IOB::getIOB() const {
    return activeInsulinUnits;
}


void IOB::decay(float decayRate) {
    activeInsulinUnits -= decayRate;
    if (activeInsulinUnits < 0.0f)
        activeInsulinUnits = 0.0f;
}

bool IOB::isActive() const {
    return activeInsulinUnits > 0.0f;
}
