#include "GearIndicator.h"
#include <Arduino.h>

GearIndicator::GearIndicator()
	: currentGear(NEUTRAL),
	  targetGear(NEUTRAL),
	  isInitialized(false),
	  isMoving(false),
	  transitionStartTime(0),
	  currentAngle(GEAR_ANGLES[NEUTRAL]),
	  startAngle(GEAR_ANGLES[NEUTRAL]),
	  targetAngle(GEAR_ANGLES[NEUTRAL]) {
}

void GearIndicator::begin() {
    Serial.println("Initializing gear indicator servo...");
    Serial.print("Servo pin: GPIO ");
    Serial.println(SERVO_PIN);

    // Configure servo with custom pulse width range for better precision
    gearServo.setPeriodHertz(50);  // Standard 50Hz servo frequency

    if (gearServo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
        Serial.println("Servo attached successfully");
    } else {
        Serial.println("ERROR: Servo attach failed!");
        return;
    }

    // Initialize to neutral position immediately (no easing on startup)
    currentAngle = GEAR_ANGLES[NEUTRAL];
    targetGear = NEUTRAL;
    currentGear = NEUTRAL;

    Serial.print("Setting servo to neutral angle: ");
    Serial.print(currentAngle);
    Serial.println(" degrees");

    gearServo.write(currentAngle);
    delay(100);  // Give servo time to move

    isInitialized = true;

    Serial.println("Gear indicator initialized successfully");
    Serial.print("Starting gear: ");
    Serial.println(getCurrentGearName());
    Serial.print("Current servo angle: ");
    Serial.println(currentAngle);
}

void GearIndicator::setGear(Gear gear) {
    if (!isInitialized) {
        Serial.println("Error: Gear indicator not initialized. Call begin() first.");
        return;
    }

    // Validate gear selection
    if (gear < REVERSE || gear > GEAR_3) {
        Serial.print("Error: Invalid gear selection: ");
        Serial.println(gear);
        return;
    }

    // If already at target gear, do nothing
    if (gear == targetGear) {
        return;
    }

    // Start transition
    targetGear = gear;
    startAngle = currentAngle;
    targetAngle = GEAR_ANGLES[gear];
    transitionStartTime = millis();
    isMoving = true;

    Serial.print("Starting transition to gear: ");
    Serial.print(GEAR_NAMES[gear]);
    Serial.print(" (");
    Serial.print(targetAngle);
    Serial.println(" degrees)");
}

void GearIndicator::setGear(int gearIndex) {
    // Convert integer index to Gear enum
    if (gearIndex >= REVERSE && gearIndex <= GEAR_3) {
        setGear(static_cast<Gear>(gearIndex));
    } else {
        Serial.print("Error: Invalid gear index: ");
        Serial.println(gearIndex);
    }
}

void GearIndicator::update() {
    if (!isInitialized || !isMoving) {
        return;
    }

    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - transitionStartTime;

    if (elapsed >= GEAR_TRANSITION_TIME_MS) {
        // Transition complete
        currentAngle = targetAngle;
        currentGear = targetGear;
        isMoving = false;

        Serial.print("Gear transition complete: ");
        Serial.println(GEAR_NAMES[currentGear]);
    } else {
        // Calculate interpolated position
        float progress = (float)elapsed / (float)GEAR_TRANSITION_TIME_MS;
        float easedProgress = easeInOutCubic(progress);
        currentAngle = startAngle + (targetAngle - startAngle) * easedProgress;
    }

    updateServoPosition();
}

float GearIndicator::easeInOutCubic(float t) {
    // Smooth easing function that starts slow, accelerates, then slows down
    // t is between 0.0 and 1.0
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        float f = (2.0f * t - 2.0f);
        return 1.0f + f * f * f / 2.0f;
    }
}

void GearIndicator::updateServoPosition() {
    if (!isInitialized) {
        Serial.println("ERROR: Servo not initialized in updateServoPosition()");
        return;
    }

    gearServo.write(currentAngle);

    // Debug output every 100ms during transitions
    static unsigned long lastDebugTime = 0;
    unsigned long now = millis();
    if (now - lastDebugTime > 100 && isMoving) {
        Serial.print("Servo angle: ");
        Serial.print(currentAngle);
        Serial.println(" degrees");
        lastDebugTime = now;
    }
}

void GearIndicator::testSequence() {
    if (!isInitialized) {
        Serial.println("Error: Gear indicator not initialized. Call begin() first.");
        return;
    }

    Serial.println("Starting gear indicator test sequence...");
    Serial.println("Note: Call update() regularly in your main loop to see smooth transitions");

    // Cycle through all gears (transitions will be handled by update() method)
    for (int i = REVERSE; i <= GEAR_3; i++) {
        setGear(static_cast<Gear>(i));

        // Wait for transition to complete
        while (isInTransition()) {
            update();
            delay(10);  // Small delay for smooth animation
        }
        delay(500);  // Hold position briefly
    }

    // Return to neutral
    setGear(NEUTRAL);
    while (isInTransition()) {
        update();
        delay(10);
    }

    Serial.println("Gear indicator test sequence complete");
}

void GearIndicator::testServoOutput() {
    if (!isInitialized) {
        Serial.println("Error: Gear indicator not initialized. Call begin() first.");
        return;
    }

    Serial.println("=== SERVO OUTPUT TEST FOR SCOPE VERIFICATION ===");
    Serial.println("This will output specific angles for scope measurement");

    // Test each gear position with clear debug output
    int testAngles[] = {0, 15, 30, 45, 60};
    const char* testNames[] = {"Reverse", "Neutral", "1st Gear", "2nd Gear", "3rd Gear"};

    for (int i = 0; i < 5; i++) {
        Serial.print("Setting servo to ");
        Serial.print(testAngles[i]);
        Serial.print(" degrees (");
        Serial.print(testNames[i]);
        Serial.println(")");

        // Set servo position directly
        gearServo.write(testAngles[i]);
        currentAngle = testAngles[i];

        Serial.println(">>> Check scope now! PWM should be active on GPIO 18 <<<");
        delay(3000);  // 3 seconds to observe on scope
    }

    // Test extreme positions for pulse width verification
    Serial.println("\nTesting minimum angle (0 degrees):");
    gearServo.write(0);
    delay(2000);

    Serial.println("Testing maximum angle (180 degrees):");
    gearServo.write(180);
    delay(2000);

    // Return to neutral
    Serial.println("Returning to neutral (15 degrees):");
    gearServo.write(15);
    currentAngle = 15;

    Serial.println("=== SERVO OUTPUT TEST COMPLETE ===");
}