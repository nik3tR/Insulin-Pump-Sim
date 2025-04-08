#include "controliq.h"

ControlIQ::ControlIQ() {}

// If BG is high, use full rate; else reduce rate.
double ControlIQ::adjustDelivery(double cgm) {
    return (cgm >= 7.0 ? 1.0 : 0.5);
}
