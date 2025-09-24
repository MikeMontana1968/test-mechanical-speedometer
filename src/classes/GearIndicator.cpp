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
    // Configure servo with custom pulse width range for better precision
    gearServo.setPeriodHertz(50);  // Standard 50Hz servo frequency
    gearServo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);

    // Initialize to neutral position immediately (no easing on startup)
    currentAngle = GEAR_ANGLES[NEUTRAL];
    gearServo.write(currentAngle);
    isInitialized = true;

    Serial.println("Gear indicator initialized");
    Serial.print("Starting gear: ");
    Serial.println(getCurrentGearName());
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
    gearServo.write(currentAngle);
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