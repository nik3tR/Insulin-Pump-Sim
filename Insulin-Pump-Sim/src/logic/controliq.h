#ifndef CONTROLIQ_H
#define CONTROLIQ_H

//--------------------------------------------------------
// MODULAR CLASSES FOR INSULIN DELIVERY
// Adjusts delivery based on CGM reading
//--------------------------------------------------------
class ControlIQ {
public:
    ControlIQ();
    double adjustDelivery(double cgm);
};

#endif // CONTROLIQ_H
