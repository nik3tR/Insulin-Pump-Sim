#include "cgmsensor.h"

CGMSensor::CGMSensor() : currentGlucoseLevel(5.5f), connected(true) {}

float CGMSensor::getGlucoseLevel() const {
    return currentGlucoseLevel;
}

void CGMSensor::updateGlucoseData(float newLevel) {
    currentGlucoseLevel = newLevel;
}

bool CGMSensor::isConnected() const {
    return connected;
}

void CGMSensor::disconnectSensor() {
    connected = false;
}

void CGMSensor::connectSensor() {
    connected = true;
}
