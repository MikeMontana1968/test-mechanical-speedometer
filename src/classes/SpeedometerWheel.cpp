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
    Serial.println("Initializing stepper motor...");
    Serial.print("Stepper pins: ");
    Serial.print(STEPPER_PIN_1); Serial.print(", ");
    Serial.print(STEPPER_PIN_2); Serial.print(", ");
    Serial.print(STEPPER_PIN_3); Serial.print(", ");
    Serial.println(STEPPER_PIN_4);
    Serial.print("Constructor pin order: ");
    Serial.print(STEPPER_PIN_1); Serial.print(", ");
    Serial.print(STEPPER_PIN_3); Serial.print(", ");
    Serial.print(STEPPER_PIN_2); Serial.print(", ");
    Serial.println(STEPPER_PIN_4);
    Serial.print("Endstop pin: GPIO ");
    Serial.println(ENDSTOP_PIN);

    pinMode(ENDSTOP_PIN, INPUT_PULLUP);
    stepper.setSpeed(STEPPER_RPM);
    currentPosition = 0;
    currentPositionFloat = 0.0;

    Serial.println("Stepper speed set to: " + String(STEPPER_RPM) + " RPM");
    Serial.println("Steps per revolution: " + String(STEPS_PER_REVOLUTION));

    // Test stepper motor with a few steps
    Serial.println("Testing stepper motor movement...");
    testStepperMotor();


    // Run manual stepper test to verify motor operation
    Serial.println("\nRunning manual stepper motor test...");
    manualStepperTest();
}

bool SpeedometerWheel::readEndstop() {
    return digitalRead(ENDSTOP_PIN) == HIGH;  // HIGH means marker is detected
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

    Serial.print("Searching for ");
    Serial.print(risingEdge ? "rising" : "falling");
    Serial.print(" edge, starting from state: ");
    Serial.println(currentState ? "TRIGGERED" : "OPEN");

    // Move until we find the edge (search up to 1.5 revolutions to be thorough)
    for (int i = 0; i < (STEPS_PER_REVOLUTION * 3 / 2); i++) {
        singleStep(clockwise);
        delay(5);  // Small delay for sensor stability

        bool newState = readEndstop();

        // Debug output every 100 steps
        if (i % 100 == 0) {
            Serial.print("Step ");
            Serial.print(i);
            Serial.print("/");
            Serial.print(STEPS_PER_REVOLUTION * 3 / 2);
            Serial.print(" - Sensor: ");
            Serial.println(newState ? "TRIGGERED" : "OPEN");
        }

        if (currentState != newState && newState == targetState) {
            Serial.print("Edge found at step ");
            Serial.print(currentPosition);
            Serial.print(" - Transition: ");
            Serial.print(currentState ? "TRIGGERED" : "OPEN");
            Serial.print(" -> ");
            Serial.println(newState ? "TRIGGERED" : "OPEN");
            return currentPosition;
        }
        currentState = newState;
    }

    Serial.println("Edge not found after 1.5 revolutions");
    return -1;  // Edge not found
}

bool SpeedometerWheel::calibrateHome() {
    Serial.println("Starting home calibration...");
    Serial.println("Looking for home marker...");

    // Check initial sensor state
    bool initialState = readEndstop();
    Serial.print("Initial sensor state: ");
    Serial.println(initialState ? "TRIGGERED" : "OPEN");

    // Verify stepper can move by testing a few steps
    Serial.println("Pre-calibration movement test...");
    for (int i = 0; i < 5; i++) {
        Serial.print("Test step ");
        Serial.print(i + 1);
        Serial.print(" - Sensor: ");
        bool state = readEndstop();
        Serial.print(state ? "TRIGGERED" : "OPEN");

        stepper.step(1);
        delay(200);  // Longer delay for observation

        bool newState = readEndstop();
        Serial.print(" -> ");
        Serial.println(newState ? "TRIGGERED" : "OPEN");
    }

    // First, try clockwise rotation to find the start of the home marker
    Serial.println("Phase 1: Finding rising edge (entering marker) - Clockwise search...");
    homeStartPosition = findEdge(true, true);  // Find rising edge (entering marker)

    if (homeStartPosition == -1) {
        Serial.println("Marker not found clockwise, trying counterclockwise...");
        homeStartPosition = findEdge(false, true);  // Try counterclockwise
    }

    if (homeStartPosition == -1) {
        Serial.println("Home marker start not found in either direction!");
        Serial.println("Troubleshooting tips:");
        Serial.println("- Ensure marker is attached to wheel");
        Serial.println("- Check endstop sensor alignment");
        Serial.println("- Verify marker can block optical sensor");
        Serial.println("- Try manually rotating wheel to see sensor transitions");
        return false;
    }

    Serial.print("Home marker starts at step: ");
    Serial.println(homeStartPosition);

    // Continue rotating to find the end of the home marker
    Serial.println("Phase 2: Finding falling edge (leaving marker)...");
    homeEndPosition = findEdge(true, false);  // Find falling edge (leaving marker)
    if (homeEndPosition == -1) {
        Serial.println("Home marker end not found!");
        Serial.println("Marker may be too wide or sensor issue occurred");
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

void SpeedometerWheel::testStepperMotor() {
    Serial.println("=== STEPPER MOTOR TEST ===");
    Serial.println("Testing stepper motor with 10 steps clockwise...");

    // Test with small number of steps to verify movement
    for (int i = 0; i < 10; i++) {
        Serial.print("Step ");
        Serial.print(i + 1);
        Serial.print("/10 - Sensor: ");
        bool sensorState = readEndstop();
        Serial.println(sensorState ? "TRIGGERED" : "OPEN");

        stepper.step(1);
        delay(100);  // Slower for observation
    }

    Serial.println("Test complete. If no sensor changes occurred, check:");
    Serial.println("- Stepper motor wiring");
    Serial.println("- Power supply to stepper driver");
    Serial.println("- Pin connections: GPIO 25,26,27,32");
    Serial.println("- ULN2003 driver board connections");
}

void SpeedometerWheel::continuousStepperTest() {
    Serial.println("\n=== CONTINUOUS STEPPER & SENSOR TEST ===");
    Serial.println("Using manual 28BYJ-48 step sequence for reliable rotation.");
    Serial.println("Watch for sensor state transitions as the wheel rotates.");
    Serial.println("Send any character via serial to stop the test.\n");

    // Set all pins as outputs
    pinMode(STEPPER_PIN_1, OUTPUT);
    pinMode(STEPPER_PIN_2, OUTPUT);
    pinMode(STEPPER_PIN_3, OUTPUT);
    pinMode(STEPPER_PIN_4, OUTPUT);

    // 28BYJ-48 step sequence (full step mode)
    int stepSequence[4][4] = {
        {1, 0, 0, 1},  // Step 1
        {1, 1, 0, 0},  // Step 2
        {0, 1, 1, 0},  // Step 3
        {0, 0, 1, 1}   // Step 4
    };

    bool lastSensorState = readEndstop();
    int stepCount = 0;
    currentPosition = 0;

    Serial.print("Starting sensor state: ");
    Serial.println(lastSensorState ? "TRIGGERED" : "OPEN");
    Serial.println("Rotating stepper motor clockwise with 200ms step timing + power saving...\n");

    while (true) {
        // Get current step in sequence
        int currentStep = stepCount % 4;

        // Apply the step pattern
        digitalWrite(STEPPER_PIN_1, stepSequence[currentStep][0]);
        digitalWrite(STEPPER_PIN_2, stepSequence[currentStep][1]);
        digitalWrite(STEPPER_PIN_3, stepSequence[currentStep][2]);
        digitalWrite(STEPPER_PIN_4, stepSequence[currentStep][3]);

        // Hold pattern for 50ms, then turn off coils to save power
        delay(50);
        digitalWrite(STEPPER_PIN_1, 0);
        digitalWrite(STEPPER_PIN_2, 0);
        digitalWrite(STEPPER_PIN_3, 0);
        digitalWrite(STEPPER_PIN_4, 0);

        stepCount++;
        currentPosition++;

        // Wrap position at full revolution
        if (currentPosition >= STEPS_PER_REVOLUTION) {
            currentPosition = 0;
        }

        // Check sensor state
        bool currentSensorState = readEndstop();

        // Report if sensor state changed
        if (currentSensorState != lastSensorState) {
            Serial.print("*** SENSOR CHANGE at step ");
            Serial.print(stepCount);
            Serial.print(" (position ");
            Serial.print(currentPosition);
            Serial.print("): ");
            Serial.print(lastSensorState ? "TRIGGERED" : "OPEN");
            Serial.print(" -> ");
            Serial.print(currentSensorState ? "TRIGGERED" : "OPEN");
            Serial.println(" ***");
            lastSensorState = currentSensorState;
        }

        // Progress report every 50 steps
        if (stepCount % 50 == 0) {
            Serial.print("Step ");
            Serial.print(stepCount);
            Serial.print(" - Position: ");
            Serial.print(currentPosition);
            Serial.print(" - Sensor: ");
            Serial.print(currentSensorState ? "TRIGGERED" : "OPEN");
            Serial.print(" - Pattern: ");
            Serial.print(stepSequence[currentStep][0]);
            Serial.print(stepSequence[currentStep][1]);
            Serial.print(stepSequence[currentStep][2]);
            Serial.println(stepSequence[currentStep][3]);
        }

        // Check for serial input to stop
        if (Serial.available()) {
            Serial.read(); // Clear the input
            Serial.println("\n*** Test stopped by user input ***");
            break;
        }

        delay(150);  // Additional 150ms rest (total 200ms per step)
    }

    // Turn off all pins
    digitalWrite(STEPPER_PIN_1, 0);
    digitalWrite(STEPPER_PIN_2, 0);
    digitalWrite(STEPPER_PIN_3, 0);
    digitalWrite(STEPPER_PIN_4, 0);

    Serial.println("=== CONTINUOUS TEST COMPLETE ===");
    Serial.print("Total steps taken: ");
    Serial.println(stepCount);
}

void SpeedometerWheel::alternativeStepperTest() {
    Serial.println("=== ALTERNATIVE STEPPER TEST ===");
    Serial.println("If the regular stepper isn't working, this might be a pin sequence issue.");
    Serial.println("The current pin order in constructor is: IN1, IN3, IN2, IN4");
    Serial.println("For 28BYJ-48, the Arduino Stepper library expects this specific order.");
    Serial.println("Let's try some manual pin control to verify hardware...");

    // Set all pins as outputs
    pinMode(STEPPER_PIN_1, OUTPUT);
    pinMode(STEPPER_PIN_2, OUTPUT);
    pinMode(STEPPER_PIN_3, OUTPUT);
    pinMode(STEPPER_PIN_4, OUTPUT);

    // Test basic pin control
    Serial.println("Testing individual pin control (should cause small movements):");

    for (int cycle = 0; cycle < 3; cycle++) {
        Serial.print("Cycle ");
        Serial.print(cycle + 1);
        Serial.println(" - Activating pins in sequence:");

        // Basic 4-step sequence
        digitalWrite(STEPPER_PIN_1, HIGH); digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW); digitalWrite(STEPPER_PIN_4, LOW);
        Serial.println("  PIN1=HIGH, others=LOW");
        delay(500);

        digitalWrite(STEPPER_PIN_1, LOW); digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW); digitalWrite(STEPPER_PIN_4, LOW);
        Serial.println("  PIN2=HIGH, others=LOW");
        delay(500);

        digitalWrite(STEPPER_PIN_1, LOW); digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH); digitalWrite(STEPPER_PIN_4, LOW);
        Serial.println("  PIN3=HIGH, others=LOW");
        delay(500);

        digitalWrite(STEPPER_PIN_1, LOW); digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW); digitalWrite(STEPPER_PIN_4, HIGH);
        Serial.println("  PIN4=HIGH, others=LOW");
        delay(500);
    }

    // Turn off all pins
    digitalWrite(STEPPER_PIN_1, LOW);
    digitalWrite(STEPPER_PIN_2, LOW);
    digitalWrite(STEPPER_PIN_3, LOW);
    digitalWrite(STEPPER_PIN_4, LOW);

    Serial.println("Manual pin test complete.");
    Serial.println("If you saw/heard the stepper move, wiring is correct.");
    Serial.println("If no movement, check:");
    Serial.println("- 5V power to ULN2003 driver");
    Serial.println("- Connections: GPIO25->IN1, GPIO26->IN2, GPIO27->IN3, GPIO32->IN4");
    Serial.println("- ULN2003 to 28BYJ-48 connection");
}


void SpeedometerWheel::manualStepperTest() {
    Serial.println("=== MANUAL STEPPER CONTROL TEST ===");
    Serial.println("This bypasses the Arduino Stepper library entirely.");
    Serial.println("Uses direct 28BYJ-48 step sequence for maximum compatibility.");
    Serial.println("You should hear/feel stepper motor movement.\n");

    // Set all pins as outputs
    pinMode(STEPPER_PIN_1, OUTPUT);
    pinMode(STEPPER_PIN_2, OUTPUT);
    pinMode(STEPPER_PIN_3, OUTPUT);
    pinMode(STEPPER_PIN_4, OUTPUT);

    // 28BYJ-48 step sequence (full step mode)
    // This is the exact sequence needed for this stepper motor
    int stepSequence[4][4] = {
        {1, 0, 0, 1},  // Step 1
        {1, 1, 0, 0},  // Step 2
        {0, 1, 1, 0},  // Step 3
        {0, 0, 1, 1}   // Step 4
    };

    Serial.println("Starting 20 steps clockwise...");

    for (int step = 0; step < 20; step++) {
        int currentStep = step % 4;

        // Apply the step pattern
        digitalWrite(STEPPER_PIN_1, stepSequence[currentStep][0]);
        digitalWrite(STEPPER_PIN_2, stepSequence[currentStep][1]);
        digitalWrite(STEPPER_PIN_3, stepSequence[currentStep][2]);
        digitalWrite(STEPPER_PIN_4, stepSequence[currentStep][3]);

        Serial.print("Step ");
        Serial.print(step + 1);
        Serial.print(" - Pattern: ");
        Serial.print(stepSequence[currentStep][0]);
        Serial.print(stepSequence[currentStep][1]);
        Serial.print(stepSequence[currentStep][2]);
        Serial.println(stepSequence[currentStep][3]);

        delay(100);  // 100ms between steps for observation
    }

    // Turn off all pins
    digitalWrite(STEPPER_PIN_1, 0);
    digitalWrite(STEPPER_PIN_2, 0);
    digitalWrite(STEPPER_PIN_3, 0);
    digitalWrite(STEPPER_PIN_4, 0);

    Serial.println("\n=== MANUAL STEPPER TEST COMPLETE ===");
    Serial.println("Results interpretation:");
    Serial.println("✓ Heard/felt movement: Hardware connections are good");
    Serial.println("✓ ULN2003 LEDs flashing: Driver getting signals");
    Serial.println("✗ No movement or sound: Check power/connections");
    Serial.println("✗ No ULN2003 LEDs: Check GPIO to ULN2003 wiring");
}