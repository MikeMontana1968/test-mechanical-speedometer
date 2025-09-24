#include "SpeedometerWheel.h"
#include <Arduino.h>

SpeedometerWheel::SpeedometerWheel()
	: stepper(STEPS_PER_REVOLUTION, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4),
	  currentPosition(0),
	  targetPosition(0),
	  homeStartPosition(0),
	  homeEndPosition(0),
	  homeMarkerWidth(0),
	  isCalibrated(false),
	  isMoving(false),
	  transitionStartTime(0),
	  currentPositionFloat(0.0),
	  startPositionFloat(0.0),
	  targetPositionFloat(0.0) {
}

void SpeedometerWheel::begin() {
    pinMode(ENDSTOP_PIN, INPUT_PULLUP);
    stepper.setSpeed(STEPPER_RPM);
    currentPosition = 0;
    currentPositionFloat = 0.0;
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
    targetPosition = (homeCenter + targetSteps) % STEPS_PER_REVOLUTION;

    // If already at target, do nothing
    if (abs(targetPosition - (int)round(currentPositionFloat)) < 2) {
        return;
    }

    // Start smooth transition
    startPositionFloat = currentPositionFloat;
    targetPositionFloat = targetPosition;

    // Handle wrap-around for shortest path
    if (abs(targetPositionFloat - startPositionFloat) > STEPS_PER_REVOLUTION / 2) {
        if (targetPositionFloat > startPositionFloat) {
            targetPositionFloat -= STEPS_PER_REVOLUTION;
        } else {
            targetPositionFloat += STEPS_PER_REVOLUTION;
        }
    }

    transitionStartTime = millis();
    isMoving = true;

    Serial.print("Starting transition to ");
    Serial.print(mph);
    Serial.print(" MPH (target position: ");
    Serial.print(targetPosition);
    Serial.println(")");
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

void SpeedometerWheel::update() {
    if (!isCalibrated || !isMoving) {
        return;
    }

    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - transitionStartTime;

    if (elapsed >= SPEED_TRANSITION_TIME_MS) {
        // Transition complete
        currentPositionFloat = targetPositionFloat;

        // Handle wrap-around
        while (currentPositionFloat >= STEPS_PER_REVOLUTION) {
            currentPositionFloat -= STEPS_PER_REVOLUTION;
        }
        while (currentPositionFloat < 0) {
            currentPositionFloat += STEPS_PER_REVOLUTION;
        }

        currentPosition = (int)round(currentPositionFloat);
        isMoving = false;

        Serial.print("Speed transition complete. Position: ");
        Serial.print(currentPosition);
        Serial.print(" (");
        Serial.print(getCurrentMPH());
        Serial.println(" MPH)");
    } else {
        // Calculate interpolated position
        float progress = (float)elapsed / (float)SPEED_TRANSITION_TIME_MS;
        float easedProgress = easeInOutCubic(progress);
        currentPositionFloat = startPositionFloat + (targetPositionFloat - startPositionFloat) * easedProgress;
    }

    updateStepperPosition();
}

float SpeedometerWheel::easeInOutCubic(float t) {
    // Smooth easing function that starts slow, accelerates, then slows down
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        float f = (2.0f * t - 2.0f);
        return 1.0f + f * f * f / 2.0f;
    }
}

void SpeedometerWheel::updateStepperPosition() {
    int targetSteps = (int)round(currentPositionFloat);

    // Handle wrap-around
    while (targetSteps >= STEPS_PER_REVOLUTION) {
        targetSteps -= STEPS_PER_REVOLUTION;
    }
    while (targetSteps < 0) {
        targetSteps += STEPS_PER_REVOLUTION;
    }

    int stepsToMove = shortestPath(currentPosition, targetSteps);

    if (stepsToMove != 0) {
        stepper.step(stepsToMove);
        currentPosition = targetSteps;
    }
}

int SpeedometerWheel::shortestPath(int from, int to) {
    int diff = (to - from + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
    if (diff > STEPS_PER_REVOLUTION / 2) {
        diff -= STEPS_PER_REVOLUTION;
    }
    return diff;
}

int SpeedometerWheel::getCurrentMPH() const {
    if (!isCalibrated) {
        return 0;
    }

    int homeCenter = (homeStartPosition + homeMarkerWidth / 2) % STEPS_PER_REVOLUTION;
    int currentPos = (int)round(currentPositionFloat);
    while (currentPos >= STEPS_PER_REVOLUTION) currentPos -= STEPS_PER_REVOLUTION;
    while (currentPos < 0) currentPos += STEPS_PER_REVOLUTION;

    int stepsFromHomeCenter = (currentPos - homeCenter + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
    if (stepsFromHomeCenter > STEPS_PER_REVOLUTION / 2) {
        stepsFromHomeCenter -= STEPS_PER_REVOLUTION;
    }

    int stepsFromZero = stepsFromHomeCenter - ZERO_MPH_OFFSET;
    return constrain(stepsFromZero / STEPS_PER_MPH, MIN_SPEED_MPH, MAX_SPEED_MPH);
}

int SpeedometerWheel::getTargetMPH() const {
    if (!isCalibrated) {
        return 0;
    }

    int homeCenter = (homeStartPosition + homeMarkerWidth / 2) % STEPS_PER_REVOLUTION;
    int stepsFromHomeCenter = (targetPosition - homeCenter + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
    if (stepsFromHomeCenter > STEPS_PER_REVOLUTION / 2) {
        stepsFromHomeCenter -= STEPS_PER_REVOLUTION;
    }

    int stepsFromZero = stepsFromHomeCenter - ZERO_MPH_OFFSET;
    return constrain(stepsFromZero / STEPS_PER_MPH, MIN_SPEED_MPH, MAX_SPEED_MPH);
}