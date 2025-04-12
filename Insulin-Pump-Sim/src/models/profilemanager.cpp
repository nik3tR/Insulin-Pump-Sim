#include "profilemanager.h"

void ProfileManager::createProfile(const Profile& profile) {
    profiles.push_back(profile);
}


const std::vector<Profile>& ProfileManager::getProfiles() const {
    return profiles;
}


Profile* ProfileManager::selectProfile(const std::string& name) {
    for (auto& p : profiles) {
        if (p.getName() == name)
            return &p;
    }
    return nullptr;
}

//Remove the profile if chosen to be deleted
void ProfileManager::deleteProfile(const std::string& name) {
    auto it = std::remove_if(profiles.begin(), profiles.end(), [&](const Profile& p) {
        return p.getName() == name;
    });
    if(it != profiles.end()){
        profiles.erase(it, profiles.end());
    }
}
