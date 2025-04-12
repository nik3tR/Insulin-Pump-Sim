#ifndef INSULINCARTRIDGE_H
#define INSULINCARTRIDGE_H

//--------------------------------------------------------
// INSULINCARTRIDGE
//--------------------------------------------------------
class InsulinCartridge {
public:
    int insulinLevel;
    bool occluded; //occlusion flag
    InsulinCartridge();
    void updateInsulinLevel(int newLevel);
    void refill();
    int getInsulinLevel() const;
    bool isOccluded() const;
    void setOcclusion(bool status);
};

#endif // INSULINCARTRIDGE_H
