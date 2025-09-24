#ifndef SPEEDOMETER_WHEEL_H
#define SPEEDOMETER_WHEEL_H

#include <Stepper.h>
#include "config.h"

class SpeedometerWheel {
private:
    Stepper stepper;
    int currentPosition;        // Current step position
    int homeStartPosition;      // Step position where home marker starts
    int homeEndPosition;        // Step position where home marker ends
    int homeMarkerWidth;        // Width of home marker in steps
    bool isCalibrated;          // Whether home calibration has been completed

    static const int ZERO_MPH_OFFSET = 256;  // Steps from home to 0 MPH position (1/8 revolution)

    bool readEndstop();
    void singleStep(bool clockwise);
    int findEdge(bool clockwise, bool risingEdge);

public:
    SpeedometerWheel();

    // Initialization and calibration
    void begin();
    bool calibrateHome();

    // Movement methods
    void moveToMPH(int mph);
    bool homeWheel();

    // Getters
    int getCurrentPosition() const { return currentPosition; }
    int getHomeMarkerWidth() const { return homeMarkerWidth; }
    bool getCalibrationStatus() const { return isCalibrated; }

    // Utility methods
    int stepsFromHome(int mph);
    int shortestPathToHome();
};

#endif // SPEEDOMETER_WHEEL_H