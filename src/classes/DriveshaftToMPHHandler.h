#ifndef DRIVESHAFT_TO_MPH_HANDLER_H
#define DRIVESHAFT_TO_MPH_HANDLER_H

#include "config.h"
#include "GearIndicator.h"
#include "SpeedometerWheel.h"
#include "DriveshaftInterruptHandler.h"

class DriveshaftToMPHHandler {
private:
    GearIndicator* gearIndicator;
    SpeedometerWheel* speedometer;
    DriveshaftInterruptHandler* driveshaftMonitor;

    // 1970 MGB Three-speed manual transmission specifications
    static const float TRANSMISSION_RATIOS[5];  // Index 0=Reverse, 1=1st, 2=2nd, 3=3rd, 4=Not used
    static constexpr float DIFFERENTIAL_RATIO = 3.9f;      // 1970 MGB differential ratio
    static constexpr float TIRE_DIAMETER_INCHES = 23.0f;   // Approximate for 165-80R13 tires

    // Internal state
    Gear currentGear;
    Gear candidateGear;          // Gear being evaluated for stability
    int currentSpeed;
    float lastDriveshaftRPM;
    float lastWheelRPM;

    // Gear stability tracking
    static const unsigned long GEAR_STABILITY_TIMEOUT_MS = 750;  // Time to confirm gear
    static constexpr float GEAR_RATIO_TOLERANCE = 0.3f;          // Tolerance for gear detection
    unsigned long lastValidGearTime;
    unsigned long candidateGearStartTime;

    // Conversion constants
    static constexpr float INCHES_PER_MILE = 63360.0f;
    static constexpr float MINUTES_PER_HOUR = 60.0f;

    // Helper methods
    Gear calculateOptimalGear(float driveshaftRPM, float wheelRPM);
    Gear evaluateGearStability(Gear detectedGear, unsigned long currentTime);
    bool isGearRatioValid(float actualRatio, Gear gear);
    int calculateSpeedFromWheelRPM(float wheelRPM);
    float calculateExpectedEngineRPM(Gear gear, float driveshaftRPM);

public:
    DriveshaftToMPHHandler(GearIndicator* gearInd, SpeedometerWheel* speedo, DriveshaftInterruptHandler* driveshaft = nullptr);

    // Main update method - call this regularly with current RPM values
    float update(float driveshaftRPM, float wheelRPM);

    // Overloaded update method that uses DriveshaftInterruptHandler for automatic driveshaft RPM
    float update(float driveshaftRPM);

    // Configuration methods
    void setDifferentialRatio(float ratio) { /* Not implemented - const for MGB */ }
    void setTireDiameter(float inches) { /* Not implemented - const for MGB */ }

    // Getters
    Gear getCurrentGear() const { return currentGear; }
    int getCurrentSpeed() const { return currentSpeed; }
    float getDifferentialRatio() const { return DIFFERENTIAL_RATIO; }
    float getTireDiameter() const { return TIRE_DIAMETER_INCHES; }
    float getTransmissionRatio(Gear gear) const;

    // Utility methods
    void printStatus();
};

#endif // DRIVESHAFT_TO_MPH_HANDLER_H