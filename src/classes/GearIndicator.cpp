#include "GearIndicator.h"
#include <Arduino.h>

GearIndicator::GearIndicator()
    : currentGear(NEUTRAL), isInitialized(false) {
}

void GearIndicator::begin() {
    // Configure servo with custom pulse width range for better precision
    gearServo.setPeriodHertz(50);  // Standard 50Hz servo frequency
    gearServo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);

    // Initialize to neutral position
    setGear(NEUTRAL);
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

    currentGear = gear;
    int targetAngle = GEAR_ANGLES[gear];

    Serial.print("Setting gear to: ");
    Serial.print(GEAR_NAMES[gear]);
    Serial.print(" (");
    Serial.print(targetAngle);
    Serial.println(" degrees)");

    gearServo.write(targetAngle);
    delay(500);  // Allow time for servo to reach position
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

void GearIndicator::testSequence() {
    if (!isInitialized) {
        Serial.println("Error: Gear indicator not initialized. Call begin() first.");
        return;
    }

    Serial.println("Starting gear indicator test sequence...");

    // Cycle through all gears
    for (int i = REVERSE; i <= GEAR_3; i++) {
        setGear(static_cast<Gear>(i));
        delay(1000);  // Hold each position for 1 second
    }

    // Return to neutral
    setGear(NEUTRAL);
    Serial.println("Gear indicator test sequence complete");
}