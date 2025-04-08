#ifndef IOB_H
#define IOB_H


//--------------------------------------------------------
// IOB (Insulin On Board)
//--------------------------------------------------------
class IOB {
public:
    float activeInsulinUnits;

    IOB();

    void updateIOB(float units);
    float getIOB() const;
};

#endif // IOB_H
