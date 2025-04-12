#include "src/logic/bolusmanager.h"
#include "bolusmanager.h"
#include <algorithm>

// Constructor
BolusManager::BolusManager(Profile* profile, IOB* iob)
    : m_profile(profile), m_iob(iob)
{ }

// Use formulas to calculate bolus insulin amount
BolusResult BolusManager::calculateStandard(double carbs,
                                            double currentBG) const
{
    // defaults
    double carbRatio        = 10.0;
    double correctionFactor = 2.0;
    double targetBG         = 6.0;

    if (m_profile) {
        carbRatio        = m_profile->getCarbRatio();
        correctionFactor = m_profile->getCorrectionFactor();
        targetBG         = m_profile->getTargetGlucose();
    }

    double carbBolus       = carbs / carbRatio;
    double correctionBolus = (currentBG > targetBG)
                                 ? (currentBG - targetBG) / correctionFactor
                                 : 0.0;
    double totalBolus      = carbBolus + correctionBolus;
    double existingIOB     = m_iob ? m_iob->getIOB() : 0.0;
    double finalBolus      = totalBolus - existingIOB;
    if (finalBolus < 0) finalBolus = 0;

    return { carbBolus, correctionBolus, totalBolus, finalBolus, existingIOB };
}

// Calculate immediate vs extended portions of bolus
ExtendedBolusParams BolusManager::calculateExtended(double totalBolus,
                                                    double immediatePct,
                                                    double extendedPct,
                                                    double durationHours) const
{
    // clamp percents
    immediatePct = std::clamp(immediatePct, 0.0, 100.0);
    extendedPct  = std::clamp(extendedPct,  0.0, 100.0);

    double immDose = totalBolus * (immediatePct / 100.0);
    double extDose = totalBolus * (extendedPct  / 100.0);
    double rate    = (durationHours > 0.0)
                      ? (extDose / durationHours)
                      : 0.0;

    return { immDose, extDose, rate };
}
