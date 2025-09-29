#include "EngineRPMInterruptHandler.h"
#include <Arduino.h>

volatile unsigned long EngineRPMInterruptHandler::enginePulseCount = 0;
volatile unsigned long EngineRPMInterruptHandler::engineLastPulseTime = 0;
EngineRPMInterruptHandler* EngineRPMInterruptHandler::instance = nullptr;

EngineRPMInterruptHandler::EngineRPMInterruptHandler(uint8_t pin)
	: gpioPin(pin),
	  lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0),
	  enabled(true) {  // Start enabled for testing/debug
    instance = this;
}

void EngineRPMInterruptHandler::begin() {
    pinMode(gpioPin, INPUT_PULLUP);

    // Initialize all counters before enabling interrupt
    unsigned long currentTime = millis();
    enginePulseCount = 0;
    engineLastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    lastCalculationTime = currentTime;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    // Enable interrupt after initialization
    attachInterrupt(digitalPinToInterrupt(gpioPin),
                   handleInterrupt,
                   FALLING);

    Serial.println("EngineRPMInterruptHandler: Initialized on GPIO " + String(gpioPin));
}

void EngineRPMInterruptHandler::handleInterrupt() {
    // Only process interrupts if monitoring is enabled
    if (!instance || !instance->enabled) {
        return;
    }

    unsigned long currentTime = millis();

    // Debounce with 10ms minimum between pulses
    if (currentTime - engineLastPulseTime > 10) {
        enginePulseCount++;
        engineLastPulseTime = currentTime;
    }
}

void EngineRPMInterruptHandler::update() {
    unsigned long currentTime = millis();

    if (currentTime - lastCalculationTime >= RPM_CALCULATION_INTERVAL_MS) {
        unsigned long currentPulseCount = enginePulseCount;
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
        } else if (currentTime - engineLastPulseTime > RPM_TIMEOUT_MS) {
            // No pulses for timeout period - engine likely stopped
            currentRPM = 0.0f;
        }

        lastPulseCountSnapshot = currentPulseCount;
        lastCalculationTime = currentTime;
    }
}

bool EngineRPMInterruptHandler::isReceivingSignal() const {
    // Basic pulse detection - shows any recent interrupt activity (for debug)
    return (millis() - engineLastPulseTime) < RPM_TIMEOUT_MS;
}

bool EngineRPMInterruptHandler::isValidSignal() const {
    // Filtered signal validation - requires stable RPM to be considered real
    // This filters out electrical noise that creates sporadic low-rate pulses
    return isReceivingSignal() && currentRPM >= MIN_STABLE_RPM;
}

void EngineRPMInterruptHandler::reset() {
    unsigned long currentTime = millis();
    enginePulseCount = 0;
    engineLastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;
    lastCalculationTime = currentTime;
}

void EngineRPMInterruptHandler::printStatus() {
    Serial.println("=== EngineRPMInterruptHandler Status ===");
    Serial.println("Current RPM: " + String(currentRPM, 1));
    Serial.println("Total Pulses: " + String(enginePulseCount));
    Serial.println("Signal Active: " + String(isReceivingSignal() ? "Yes" : "No"));
    Serial.println("Valid Signal: " + String(isValidSignal() ? "Yes" : "No"));
    Serial.println("Last Pulse: " + String(millis() - engineLastPulseTime) + "ms ago");
    Serial.println("Enabled: " + String(enabled ? "Yes" : "No"));
}

void EngineRPMInterruptHandler::setEnabled(bool enable) {
    enabled = enable;
    if (!enable) {
        // When disabling, reset all counters to prevent stale readings
        reset();
    }
}