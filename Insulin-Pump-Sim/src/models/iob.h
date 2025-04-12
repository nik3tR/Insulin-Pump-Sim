#ifndef IOB_H
#define IOB_H

//--------------------------------------------------------
// IOB
//--------------------------------------------------------
class IOB {
public:
    float activeInsulinUnits;
    IOB();
    void updateIOB(float units);
    float getIOB() const;

    void decay(float decayRate = 0.5f);  // Default decay per tick
    bool isActive() const;               // Optional helper

};

#endif // IOB_H
