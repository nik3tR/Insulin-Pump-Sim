#ifndef BOLUSMANAGER_H
#define BOLUSMANAGER_H


#include "src/models/profile.h"
#include "src/models/iob.h"

//--------------------------------------------------------
// BolusManager: Calculates bolus values based on meal info and profile
//--------------------------------------------------------
class BolusManager {
public:
    BolusManager(Profile* profile, IOB* iob);

    double computeFinalBolus(double carbs, double currentBG);
    void computeExtendedBolus(double finalBolus, double immediatePercentage,
                              double &immediateDose, double &extendedDose);

private:
    Profile* m_profile;
    IOB* m_iob;
};


#endif // BOLUSMANAGER_H
