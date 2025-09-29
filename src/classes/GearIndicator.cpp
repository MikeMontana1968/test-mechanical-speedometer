#include "GearIndicator.h"
#include <Arduino.h>

// 1970 MGB Three-speed manual transmission ratios
const float GearIndicator::TRANSMISSION_RATIOS[5] = {
    3.44f,  // Reverse gear
    3.44f,  // 1st gear
    2.21f,  // 2nd gear
    1.37f,  // 3rd gear
    1.0f    // Not used (placeholder)
};

GearIndicator::GearIndicator(int pin)
	: servoPin(pin),
	  currentGear(NEUTRAL),
	  targetGear(NEUTRAL),
	  isInitialized(false),
	  isMoving(false),
	  transitionStartTime(0),
	  currentAngle(GEAR_ANGLES[NEUTRAL]),
	  startAngle(GEAR_ANGLES[NEUTRAL]),
	  targetAngle(GEAR_ANGLES[NEUTRAL]),
	  candidateGear(NEUTRAL),
	  candidateGearStartTime(0),
	  lastValidGearTime(0) {
}

void GearIndicator::begin() {
    Serial.println("Initializing gear indicator servo...");
    Serial.print("Servo pin: GPIO ");
    Serial.println(servoPin);

    // Configure servo with custom pulse width range for better precision
    gearServo.setPeriodHertz(50);  // Standard 50Hz servo frequency

    if (gearServo.attach(servoPin, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
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

        Serial.print(">>> Check scope now! PWM should be active on GPIO ");
        Serial.print(servoPin);
        Serial.println(" <<<");
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

void GearIndicator::updateGearFromRPM(float engineRPM, float speedMPH) {
    if (!isInitialized) {
        return;
    }

    unsigned long currentTime = millis();

    // Calculate probable gear based on engine RPM and speed
    Gear detectedGear = calculateGearFromRPMRatio(engineRPM, speedMPH);

    // Evaluate gear stability with timing logic
    Gear confirmedGear = evaluateGearStability(detectedGear, currentTime);

    // Update gear if confirmed gear changed
    if (confirmedGear != currentGear) {
        setGear(confirmedGear);

        Serial.print("Gear detected: ");
        Serial.print(GEAR_NAMES[confirmedGear]);
        Serial.print(" (Engine: ");
        Serial.print(engineRPM, 0);
        Serial.print(" RPM, Speed: ");
        Serial.print(speedMPH, 1);
        Serial.println(" MPH)");
    }
}

Gear GearIndicator::calculateGearFromRPMRatio(float engineRPM, float speedMPH) {
    // Handle special cases - low RPM or speed indicates neutral or stopped
    if (engineRPM < 500.0f || speedMPH < 1.0f) {
        return NEUTRAL;
    }

    // Calculate wheel RPM from speed
    float tireCircumference = PI * TIRE_DIAMETER_INCHES;
    float wheelRPM = (speedMPH * INCHES_PER_MILE) / (tireCircumference * MINUTES_PER_HOUR);

    // Calculate driveshaft RPM from wheel RPM
    float driveshaftRPM = wheelRPM * DIFFERENTIAL_RATIO;

    // Calculate actual transmission ratio
    float actualRatio = engineRPM / driveshaftRPM;

    // Check reverse gear (requires additional logic - could be negative speed or user input)
    // For now, skip reverse detection as it requires more context

    // Check forward gears 1-3
    for (int i = GEAR_1; i <= GEAR_3; i++) {
        if (isGearRatioValid(actualRatio, static_cast<Gear>(i))) {
            return static_cast<Gear>(i);
        }
    }

    // No valid gear ratio found - likely shifting, clutch disengaged, or neutral
    return NEUTRAL;
}

bool GearIndicator::isGearRatioValid(float actualRatio, Gear gear) {
    if (gear < REVERSE || gear > GEAR_3) {
        return false;
    }

    float expectedRatio = TRANSMISSION_RATIOS[gear];
    float difference = abs(actualRatio - expectedRatio);

    return difference <= GEAR_RATIO_TOLERANCE;
}

Gear GearIndicator::evaluateGearStability(Gear detectedGear, unsigned long currentTime) {
    // If detected gear matches candidate, continue timing
    if (detectedGear == candidateGear) {
        // Check if gear has been stable long enough
        if (currentTime - candidateGearStartTime >= GEAR_STABILITY_TIMEOUT_MS) {
            lastValidGearTime = currentTime;
            return candidateGear;  // Confirm this gear
        }
        // Still timing this candidate gear
        return currentGear;  // Keep current gear until confirmed
    }

    // New gear detected - start timing
    candidateGear = detectedGear;
    candidateGearStartTime = currentTime;

    // If we haven't had a valid gear for too long, default to neutral
    if (currentTime - lastValidGearTime > GEAR_STABILITY_TIMEOUT_MS * 2) {
        return NEUTRAL;
    }

    // Keep current gear until new one is confirmed
    return currentGear;
}

float GearIndicator::calculateExpectedEngineRPM(Gear gear, float speedMPH) {
    if (gear == NEUTRAL || speedMPH <= 0) {
        return 0.0f;
    }

    // Calculate wheel RPM from speed
    float tireCircumference = PI * TIRE_DIAMETER_INCHES;
    float wheelRPM = (speedMPH * INCHES_PER_MILE) / (tireCircumference * MINUTES_PER_HOUR);

    // Calculate driveshaft RPM from wheel RPM
    float driveshaftRPM = wheelRPM * DIFFERENTIAL_RATIO;

    // Calculate expected engine RPM
    return driveshaftRPM * TRANSMISSION_RATIOS[gear];
}