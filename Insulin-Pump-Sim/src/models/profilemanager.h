#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include "profile.h"
#include <vector>
#include <algorithm>

//--------------------------------------------------------
// PROFILE MANAGER
//--------------------------------------------------------
class ProfileManager {
private:
    std::vector<Profile> profiles;
public:
    //Create profiel
    void createProfile(const Profile& profile);
    const std::vector<Profile>& getProfiles() const;
    Profile* selectProfile(const std::string& name);
    //Remove Profile
    void deleteProfile(const std::string& name);
};

#endif // PROFILEMANAGER_H
