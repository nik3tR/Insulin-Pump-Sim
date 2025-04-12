#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <iostream>

//--------------------------------------------------------
// PROFILE
//--------------------------------------------------------
class Profile {
private:
    std::string name;
    float basalRate;
    float carbRatio;
    float correctionFactor;
    float targetGlucose;
public:
    Profile(const std::string& n, float basal, float carb, float correction, float target);
    std::string getName() const;
    float getBasalRate() const;
    float getCarbRatio() const;
    float getCorrectionFactor() const;
    float getTargetGlucose() const;
    // Update profile settings (Use Case 3: Personal Profiles - Update)
    void updateSettings(float basal, float carb, float correction, float target);
};

#endif // PROFILE_H
