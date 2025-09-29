#include "DriveshaftInterruptHandler.h"
#include <Arduino.h>

volatile unsigned long DriveshaftInterruptHandler::driveshaftPulseCount = 0;
volatile unsigned long DriveshaftInterruptHandler::driveshaftLastPulseTime = 0;
DriveshaftInterruptHandler* DriveshaftInterruptHandler::instance = nullptr;

DriveshaftInterruptHandler::DriveshaftInterruptHandler(uint8_t pin)
	: gpioPin(pin),
	  lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0),
	  enabled(true) {  // Start enabled for testing/debug
    instance = this;
}

void DriveshaftInterruptHandler::begin() {
    pinMode(gpioPin, INPUT_PULLUP);

    // Initialize all counters before enabling interrupt
    unsigned long currentTime = millis();
    driveshaftPulseCount = 0;
    driveshaftLastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    lastCalculationTime = currentTime;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    // Enable interrupt after initialization
    attachInterrupt(digitalPinToInterrupt(gpioPin),
                   handleInterrupt,
                   FALLING);

    Serial.println("DriveshaftInterruptHandler: Initialized on GPIO " + String(gpioPin));
}

void DriveshaftInterruptHandler::handleInterrupt() {
    // Only process interrupts if monitoring is enabled
    if (!instance || !instance->enabled) {
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - driveshaftLastPulseTime > 10) {
        driveshaftPulseCount++;
        driveshaftLastPulseTime = currentTime;
    }
}

void DriveshaftInterruptHandler::update() {
    unsigned long currentTime = millis();

    if (currentTime - lastCalculationTime >= RPM_CALCULATION_INTERVAL_MS) {
        unsigned long currentPulseCount = driveshaftPulseCount;
        unsigned long actualInterval = currentTime - lastCalculationTime;

        // Handle potential counter overflow/underflow
        unsigned long pulsesInInterval = 0;
        if (currentPulseCount >= lastPulseCountSnapshot) {
            // Normal case: counter incremented
            pulsesInInterval = currentPulseCount - lastPulseCountSnapshot;
        } else {
            // Counter reset or overflow - assume small number of pulses
            pulsesInInterval = currentPulseCount;
        }

        if (pulsesInInterval > 0 && actualInterval > 0) {
            float pulsesPerMinute = (float)pulsesInInterval * (60000.0f / (float)actualInterval);

            // Apply bounds checking
            if (pulsesPerMinute < MIN_RPM_THRESHOLD) {
                currentRPM = 0.0f;
            } else if (pulsesPerMinute > MAX_RPM_THRESHOLD) {
                // Unrealistic RPM - likely calculation error, keep previous value
                // (silently ignore unrealistic values)
            } else {
                currentRPM = pulsesPerMinute;
            }
        } else if (currentTime - driveshaftLastPulseTime > RPM_TIMEOUT_MS) {
            currentRPM = 0.0f;
        }

        lastPulseCountSnapshot = currentPulseCount;
        lastCalculationTime = currentTime;
    }
}

bool DriveshaftInterruptHandler::isReceivingSignal() const {
    // Basic pulse detection - shows any recent interrupt activity (for debug)
    return (millis() - driveshaftLastPulseTime) < RPM_TIMEOUT_MS;
}

bool DriveshaftInterruptHandler::isValidSignal() const {
    // Filtered signal validation - requires stable RPM to be considered real
    // This filters out electrical noise that creates sporadic low-rate pulses
    return isReceivingSignal() && currentRPM >= MIN_STABLE_RPM;
}

void DriveshaftInterruptHandler::reset() {
    unsigned long currentTime = millis();
    driveshaftPulseCount = 0;
    driveshaftLastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;
    lastCalculationTime = currentTime;
}

void DriveshaftInterruptHandler::printStatus() {
    Serial.println("=== DriveshaftInterruptHandler Status ===");
    Serial.println("Current RPM: " + String(currentRPM, 1));
    Serial.println("Total Pulses: " + String(driveshaftPulseCount));
    Serial.println("Signal Active: " + String(isReceivingSignal() ? "Yes" : "No"));
    Serial.println("Valid Signal: " + String(isValidSignal() ? "Yes" : "No"));
    Serial.println("Last Pulse: " + String(millis() - driveshaftLastPulseTime) + "ms ago");
    Serial.println("Enabled: " + String(enabled ? "Yes" : "No"));
}

void DriveshaftInterruptHandler::setEnabled(bool enable) {
    enabled = enable;
    if (!enable) {
        // When disabling, reset all counters to prevent stale readings
        reset();
    }
}