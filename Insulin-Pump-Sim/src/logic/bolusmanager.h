#ifndef BOLUSMANAGER_H
#define BOLUSMANAGER_H

#include "src/models/profile.h"
#include "src/models/iob.h"

//--------------------------------------------------------
// BolusManager: Calculates bolus values based on meal info and profile
//--------------------------------------------------------
struct BolusResult {
    double carbBolus;
    double correctionBolus;
    double totalBolus;
    double finalBolus;
    double existingIOB;
};

struct ExtendedBolusParams {
    double immediateDose;
    double extendedDose;
    double ratePerHour;
};

/// Encapsulates both standard and extended bolus calculations
class BolusManager {
public:
    BolusManager(Profile* profile, IOB* iob);

    /// Compute carb‑+correction‑bolus minus IOB
    BolusResult calculateStandard(double carbs, double currentBG) const;

    /// Split a total bolus into immediate vs. extended over duration
    ExtendedBolusParams calculateExtended(double totalBolus,
                                          double immediatePct,
                                          double extendedPct,
                                          double durationHours) const;

private:
    Profile* m_profile;
    IOB*     m_iob;
};

#endif // BOLUSMANAGER_H
