#ifndef BATTERY_H
#define BATTERY_H

//--------------------------------------------------------
// BATTERY
//--------------------------------------------------------
class Battery {
public:
    int level;
    Battery();
    //reduce charge by 10 each time
    void discharge();
    int getStatus() const;
    //charge the battery
    void charge();
};

#endif // BATTERY_H
