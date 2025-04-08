#ifndef CGMSENSOR_H
#define CGMSENSOR_H

//--------------------------------------------------------
// CGMSENSOR
//--------------------------------------------------------
class CGMSensor {
public:
    float currentGlucoseLevel;
    bool connected;

    CGMSensor();

    float getGlucoseLevel() const;
    void updateGlucoseData(float newLevel);

    bool isConnected() const;
    void disconnectSensor();
    void connectSensor();
};


#endif // CGMSENSOR_H
