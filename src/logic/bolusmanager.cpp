#include "bolusmanager.h"

// Constructor
BolusManager::BolusManager(Profile* profile, IOB* iob)
    : m_profile(profile), m_iob(iob) {}

// Use formulas to calculate bolus insulin amount
double BolusManager::computeFinalBolus(double carbs, double currentBG) {
    double carbRatio = 10.0, correctionFactor = 2.0, targetBG = 6.0;

    if (m_profile) {
        carbRatio = m_profile->getCarbRatio();
        correctionFactor = m_profile->getCorrectionFactor();
        targetBG = m_profile->getTargetGlucose();
    }

    double carbBolus = carbs / carbRatio;
    double correctionBolus = (currentBG > targetBG) ? (currentBG - targetBG) / correctionFactor : 0.0;
    double totalBolus = carbBolus + correctionBolus;
    double iob = (m_iob) ? m_iob->getIOB() : 0.0;
    double finalBolus = totalBolus - iob;
    return finalBolus < 0 ? 0 : finalBolus;
}

// Calculate immediate vs extended portions of bolus
void BolusManager::computeExtendedBolus(double finalBolus, double immediatePercentage,
                                        double &immediateDose, double &extendedDose) {
    immediateDose = finalBolus * immediatePercentage / 100.0;
    extendedDose = finalBolus - immediateDose;
}
