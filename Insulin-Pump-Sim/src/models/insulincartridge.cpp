#include "insulincartridge.h"

InsulinCartridge::InsulinCartridge() : insulinLevel(300), occluded(false) {}

void InsulinCartridge::updateInsulinLevel(int newLevel) {
    insulinLevel = newLevel;
}

void InsulinCartridge::refill() {
    insulinLevel = 300; //Default Insulin
}

int InsulinCartridge::getInsulinLevel() const {
    return insulinLevel;
}

bool InsulinCartridge::isOccluded() const {
    return occluded;
}

void InsulinCartridge::setOcclusion(bool status) {
    occluded = status;
}
