#ifndef SPEEDOMETER_WHEEL_H
#define SPEEDOMETER_WHEEL_H

#include <Stepper.h>
#include <cmath>
#include "config.h"

class SpeedometerWheel {
private:
    Stepper stepper;
    int currentPosition;        // Current step position
    int targetPosition;         // Target step position for smooth transitions
    int homeStartPosition;      // Step position where home marker starts
    int homeEndPosition;        // Step position where home marker ends
    int homeMarkerWidth;        // Width of home marker in steps
    bool isCalibrated;          // Whether home calibration has been completed
    bool isMoving;              // Whether wheel is currently transitioning

    // Smooth movement configuration
    static const unsigned long SPEED_TRANSITION_TIME_MS = 1200;  // Time to complete speed change
    unsigned long transitionStartTime;
    float currentPositionFloat;
    float startPositionFloat;
    float targetPositionFloat;

    static const int ZERO_MPH_OFFSET = 256;  // Steps from home to 0 MPH position (1/8 revolution)

    // Private helper methods
    bool readEndstop();
    void singleStep(bool clockwise);
    int findEdge(bool clockwise, bool risingEdge);
    float easeInOutCubic(float t);
    void updateStepperPosition();
    int shortestPath(int from, int to);

public:
    SpeedometerWheel();

    // Initialization and calibration
    void begin();
    bool calibrateHome();

    // Update method - call this regularly in your main loop
    void update();

    // Movement methods
    void moveToMPH(int mph);
    bool homeWheel();

    // Getters
    int getCurrentPosition() const { return (int)round(currentPositionFloat); }
    int getTargetPosition() const { return targetPosition; }
    int getCurrentMPH() const;
    int getTargetMPH() const;
    int getHomeMarkerWidth() const { return homeMarkerWidth; }
    bool getCalibrationStatus() const { return isCalibrated; }
    bool isInTransition() const { return isMoving; }

    // Utility methods
    int stepsFromHome(int mph);
    int shortestPathToHome();
    void testStepperMotor();         // Test stepper motor functionality
    void continuousStepperTest();    // Continuous stepper rotation with sensor monitoring
};

#endif // SPEEDOMETER_WHEEL_H