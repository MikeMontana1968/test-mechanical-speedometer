#ifndef GEAR_INDICATOR_H
#define GEAR_INDICATOR_H

#include <ESP32Servo.h>
#include "config.h"

class GearIndicator {
private:
    Servo gearServo;
    int servoPin;
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

    // 1970 MGB vehicle specifications for gear calculation
    static const float TRANSMISSION_RATIOS[5];  // Index 0=Reverse, 1=1st, 2=2nd, 3=3rd, 4=Not used
    static constexpr float DIFFERENTIAL_RATIO = 3.9f;      // 1970 MGB differential ratio
    static constexpr float TIRE_DIAMETER_INCHES = 23.0f;   // Approximate for 165-80R13 tires
    static constexpr float GEAR_RATIO_TOLERANCE = 0.25f;   // Tolerance for gear detection
    static constexpr float INCHES_PER_MILE = 63360.0f;
    static constexpr float MINUTES_PER_HOUR = 60.0f;

    // Gear stability tracking
    static const unsigned long GEAR_STABILITY_TIMEOUT_MS = 750;  // Time to confirm gear change
    Gear candidateGear;          // Gear being evaluated for stability
    unsigned long candidateGearStartTime;
    unsigned long lastValidGearTime;

    // Private helper methods
    float easeInOutCubic(float t);
    void updateServoPosition();
    Gear calculateGearFromRPMRatio(float engineRPM, float speedMPH);
    bool isGearRatioValid(float actualRatio, Gear gear);
    Gear evaluateGearStability(Gear detectedGear, unsigned long currentTime);
    float calculateExpectedEngineRPM(Gear gear, float speedMPH);

public:
    GearIndicator(int pin);

    // Initialization
    void begin();

    // Update method - call this regularly in your main loop
    void update();

    // Gear control methods
    void setGear(Gear gear);
    void setGear(int gearIndex);
    void updateGearFromRPM(float engineRPM, float speedMPH);  // Automatically detect and set gear

    // Getters
    Gear getCurrentGear() const { return currentGear; }
    Gear getTargetGear() const { return targetGear; }
    const char* getCurrentGearName() const { return GEAR_NAMES[currentGear]; }
    const char* getTargetGearName() const { return GEAR_NAMES[targetGear]; }
    int getCurrentGearAngle() const { return GEAR_ANGLES[currentGear]; }
    float getCurrentAngle() const { return currentAngle; }
    bool isInTransition() const { return isMoving; }

    // Utility methods
    void testSequence();     // Cycles through all gears for testing
    void testServoOutput();  // Immediate servo test for scope verification
};

#endif // GEAR_INDICATOR_H