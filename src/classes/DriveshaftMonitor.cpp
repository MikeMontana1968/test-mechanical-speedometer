#include "DriveshaftMonitor.h"
#include <Arduino.h>

volatile unsigned long DriveshaftMonitor::pulseCount = 0;
volatile unsigned long DriveshaftMonitor::lastPulseTime = 0;
DriveshaftMonitor* DriveshaftMonitor::instance = nullptr;

DriveshaftMonitor::DriveshaftMonitor(uint8_t pin)
	: gpioPin(pin),
	  lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0),
	  enabled(true) {  // Start enabled for testing/debug
    instance = this;
}

void DriveshaftMonitor::begin() {
    pinMode(gpioPin, INPUT_PULLUP);

    // Initialize all counters before enabling interrupt
    unsigned long currentTime = millis();
    pulseCount = 0;
    lastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    lastCalculationTime = currentTime;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    // Enable interrupt after initialization
    attachInterrupt(digitalPinToInterrupt(gpioPin),
                   handleInterrupt,
                   FALLING);

    Serial.println("DriveshaftMonitor: Initialized on GPIO " + String(gpioPin));
}

void DriveshaftMonitor::handleInterrupt() {
    // Only process interrupts if monitoring is enabled
    if (!instance || !instance->enabled) {
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - lastPulseTime > 10) {
        pulseCount++;
        lastPulseTime = currentTime;
    }
}

void DriveshaftMonitor::update() {
    unsigned long currentTime = millis();

    if (currentTime - lastCalculationTime >= RPM_CALCULATION_INTERVAL_MS) {
        unsigned long currentPulseCount = pulseCount;
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
        } else if (currentTime - lastPulseTime > RPM_TIMEOUT_MS) {
            currentRPM = 0.0f;
        }

        lastPulseCountSnapshot = currentPulseCount;
        lastCalculationTime = currentTime;
    }
}

bool DriveshaftMonitor::isReceivingSignal() const {
    // Basic pulse detection - shows any recent interrupt activity (for debug)
    return (millis() - lastPulseTime) < RPM_TIMEOUT_MS;
}

bool DriveshaftMonitor::isValidSignal() const {
    // Filtered signal validation - requires stable RPM to be considered real
    // This filters out electrical noise that creates sporadic low-rate pulses
    return isReceivingSignal() && currentRPM >= MIN_STABLE_RPM;
}

void DriveshaftMonitor::reset() {
    unsigned long currentTime = millis();
    pulseCount = 0;
    lastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;
    lastCalculationTime = currentTime;
}

void DriveshaftMonitor::printStatus() {
    Serial.println("=== DriveshaftMonitor Status ===");
    Serial.println("Current RPM: " + String(currentRPM, 1));
    Serial.println("Total Pulses: " + String(pulseCount));
    Serial.println("Signal Active: " + String(isReceivingSignal() ? "Yes" : "No"));
    Serial.println("Valid Signal: " + String(isValidSignal() ? "Yes" : "No"));
    Serial.println("Last Pulse: " + String(millis() - lastPulseTime) + "ms ago");
    Serial.println("Enabled: " + String(enabled ? "Yes" : "No"));
}

void DriveshaftMonitor::setEnabled(bool enable) {
    enabled = enable;
    if (!enable) {
        // When disabling, reset all counters to prevent stale readings
        reset();
    }
}