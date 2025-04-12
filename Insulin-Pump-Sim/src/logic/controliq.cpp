#include "controliq.h"

ControlIQ::ControlIQ() {}

double ControlIQ::adjustDelivery(double cgm) {
    //if BG is high, full rate else if low, reduce rate.
    return (cgm >= 7.0 ? 1.0 : 0.5);
}
