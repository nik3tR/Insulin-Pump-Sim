#ifndef BATTERY_H
#define BATTERY_H


#pragma once

class Battery {
public:
    int level;
    Battery();

    void discharge();
    int getStatus() const;
    void charge();
};


#endif // BATTERY_H
