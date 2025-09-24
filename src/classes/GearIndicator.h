#ifndef GEAR_INDICATOR_H
#define GEAR_INDICATOR_H

#include <ESP32Servo.h>
#include "config.h"

class GearIndicator {
private:
    Servo gearServo;
    Gear currentGear;
    Gear targetGear;
    bool isInitialized;
    bool isMoving;

    // Servo configuration
    static const int SERVO_MIN_PULSE = 500;   // Minimum pulse width in microseconds
    static const int SERVO_MAX_PULSE = 2500;  // Maximum pulse width in microseconds

    // Easing configuration
    static const unsigned long GEAR_TRANSITION_TIME_MS = 800;  // Time to complete gear change
    unsigned long transitionStartTime;
    float currentAngle;
    float startAngle;
    float targetAngle;

    // Private helper methods
    float easeInOutCubic(float t);
    void updateServoPosition();

public:
    GearIndicator();

    // Initialization
    void begin();

    // Update method - call this regularly in your main loop
    void update();

    // Gear control methods
    void setGear(Gear gear);
    void setGear(int gearIndex);

    // Getters
    Gear getCurrentGear() const { return currentGear; }
    Gear getTargetGear() const { return targetGear; }
    const char* getCurrentGearName() const { return GEAR_NAMES[currentGear]; }
    const char* getTargetGearName() const { return GEAR_NAMES[targetGear]; }
    int getCurrentGearAngle() const { return GEAR_ANGLES[currentGear]; }
    float getCurrentAngle() const { return currentAngle; }
    bool isInTransition() const { return isMoving; }

    // Utility methods
    void testSequence();  // Cycles through all gears for testing
};

#endif // GEAR_INDICATOR_H