#include "SpeedometerWheel.h"
#include <Arduino.h>

SpeedometerWheel::SpeedometerWheel()
    : stepper(STEPS_PER_REVOLUTION, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4),
      currentPosition(0),
      homeStartPosition(0),
      homeEndPosition(0),
      homeMarkerWidth(0),
      isCalibrated(false) {
}

void SpeedometerWheel::begin() {
    pinMode(ENDSTOP_PIN, INPUT_PULLUP);
    stepper.setSpeed(STEPPER_RPM);
    currentPosition = 0;
}

bool SpeedometerWheel::readEndstop() {
    return digitalRead(ENDSTOP_PIN) == LOW;  // LOW means marker is detected
}

void SpeedometerWheel::singleStep(bool clockwise) {
    stepper.step(clockwise ? 1 : -1);
    currentPosition += clockwise ? 1 : -1;

    // Wrap around at full revolution
    if (currentPosition >= STEPS_PER_REVOLUTION) {
        currentPosition = 0;
    } else if (currentPosition < 0) {
        currentPosition = STEPS_PER_REVOLUTION - 1;
    }
}

int SpeedometerWheel::findEdge(bool clockwise, bool risingEdge) {
    bool targetState = risingEdge;  // true for rising edge (entering marker), false for falling edge (leaving marker)
    bool currentState = readEndstop();

    // Move until we find the edge
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        singleStep(clockwise);
        delay(5);  // Small delay for sensor stability

        bool newState = readEndstop();
        if (currentState != newState && newState == targetState) {
            return currentPosition;
        }
        currentState = newState;
    }

    return -1;  // Edge not found
}

bool SpeedometerWheel::calibrateHome() {
    Serial.println("Starting home calibration...");

    // First, move to find the start of the home marker
    homeStartPosition = findEdge(true, true);  // Find rising edge (entering marker)
    if (homeStartPosition == -1) {
        Serial.println("Home marker start not found!");
        return false;
    }

    Serial.print("Home marker starts at step: ");
    Serial.println(homeStartPosition);

    // Continue rotating to find the end of the home marker
    homeEndPosition = findEdge(true, false);  // Find falling edge (leaving marker)
    if (homeEndPosition == -1) {
        Serial.println("Home marker end not found!");
        return false;
    }

    Serial.print("Home marker ends at step: ");
    Serial.println(homeEndPosition);

    // Calculate marker width
    homeMarkerWidth = (homeEndPosition - homeStartPosition + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;

    Serial.print("Home marker width: ");
    Serial.print(homeMarkerWidth);
    Serial.println(" steps");

    // Position at center of home marker
    int centerOffset = homeMarkerWidth / 2;
    int targetPosition = (homeStartPosition + centerOffset) % STEPS_PER_REVOLUTION;

    // Move to center of home marker
    int stepsToMove = (targetPosition - currentPosition + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
    if (stepsToMove > STEPS_PER_REVOLUTION / 2) {
        stepsToMove -= STEPS_PER_REVOLUTION;  // Go the shorter way
    }

    stepper.step(stepsToMove);
    currentPosition = targetPosition;

    isCalibrated = true;
    Serial.println("Home calibration complete!");
    return true;
}

void SpeedometerWheel::moveToMPH(int mph) {
    if (!isCalibrated) {
        Serial.println("Error: Wheel not calibrated. Call calibrateHome() first.");
        return;
    }

    // Constrain mph to valid range
    mph = constrain(mph, MIN_SPEED_MPH, MAX_SPEED_MPH);

    // Calculate target position
    int targetSteps = stepsFromHome(mph);
    int homeCenter = (homeStartPosition + homeMarkerWidth / 2) % STEPS_PER_REVOLUTION;
    int targetPosition = (homeCenter + targetSteps) % STEPS_PER_REVOLUTION;

    // Calculate shortest path
    int stepsToMove = (targetPosition - currentPosition + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
    if (stepsToMove > STEPS_PER_REVOLUTION / 2) {
        stepsToMove -= STEPS_PER_REVOLUTION;  // Go the shorter way
    }

    Serial.print("Moving to ");
    Serial.print(mph);
    Serial.print(" MPH (");
    Serial.print(stepsToMove);
    Serial.println(" steps)");

    stepper.step(stepsToMove);
    currentPosition = targetPosition;
}

bool SpeedometerWheel::homeWheel() {
    if (!isCalibrated) {
        return calibrateHome();
    }

    // Calculate shortest path to home center
    int homeCenter = (homeStartPosition + homeMarkerWidth / 2) % STEPS_PER_REVOLUTION;
    int stepsToMove = shortestPathToHome();

    Serial.print("Homing wheel (");
    Serial.print(stepsToMove);
    Serial.println(" steps)");

    stepper.step(stepsToMove);
    currentPosition = homeCenter;

    return true;
}

int SpeedometerWheel::stepsFromHome(int mph) {
    // Calculate steps from home center to MPH position
    // Add ZERO_MPH_OFFSET to account for 0 MPH position being offset from home
    return ZERO_MPH_OFFSET + (mph * STEPS_PER_MPH);
}

int SpeedometerWheel::shortestPathToHome() {
    if (!isCalibrated) {
        return 0;
    }

    int homeCenter = (homeStartPosition + homeMarkerWidth / 2) % STEPS_PER_REVOLUTION;
    int stepsToMove = (homeCenter - currentPosition + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;

    // Choose shortest direction
    if (stepsToMove > STEPS_PER_REVOLUTION / 2) {
        stepsToMove -= STEPS_PER_REVOLUTION;  // Go the shorter way (negative = counterclockwise)
    }

    return stepsToMove;
}