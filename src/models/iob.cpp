#include "iob.h"

IOB::IOB() : activeInsulinUnits(0.0f) {}

void IOB::updateIOB(float units) {
    activeInsulinUnits = units;
}

float IOB::getIOB() const {
    return activeInsulinUnits;
}

