#include "profile.h"

#include <iostream>

Profile::Profile(const std::string& n, float basal, float carb, float correction, float target)
    : name(n), basalRate(basal), carbRatio(carb), correctionFactor(correction), targetGlucose(target) {}

std::string Profile::getName() const { return name; }
float Profile::getBasalRate() const { return basalRate; }
float Profile::getCarbRatio() const { return carbRatio; }
float Profile::getCorrectionFactor() const { return correctionFactor; }
float Profile::getTargetGlucose() const { return targetGlucose; }

void Profile::updateSettings(float basal, float carb, float correction, float target) {
    basalRate = basal;
    carbRatio = carb;
    correctionFactor = correction;
    targetGlucose = target;
    std::cout << "Updated profile: " << name << "\n";
}

