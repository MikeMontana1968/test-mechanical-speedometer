#ifndef GEAR_INDICATOR_H
#define GEAR_INDICATOR_H

#include <ESP32Servo.h>
#include "config.h"

class GearIndicator {
private:
    Servo gearServo;
    Gear currentGear;
    bool isInitialized;

    static const int SERVO_MIN_PULSE = 500;   // Minimum pulse width in microseconds
    static const int SERVO_MAX_PULSE = 2500;  // Maximum pulse width in microseconds

public:
    GearIndicator();

    // Initialization
    void begin();

    // Gear control methods
    void setGear(Gear gear);
    void setGear(int gearIndex);

    // Getters
    Gear getCurrentGear() const { return currentGear; }
    const char* getCurrentGearName() const { return GEAR_NAMES[currentGear]; }
    int getCurrentGearAngle() const { return GEAR_ANGLES[currentGear]; }

    // Utility methods
    void testSequence();  // Cycles through all gears for testing
};

#endif // GEAR_INDICATOR_H