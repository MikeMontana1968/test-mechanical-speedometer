#include "EngineRPMMonitor.h"
#include <Arduino.h>

volatile unsigned long EngineRPMMonitor::pulseCount = 0;
volatile unsigned long EngineRPMMonitor::lastPulseTime = 0;
EngineRPMMonitor* EngineRPMMonitor::instance = nullptr;

EngineRPMMonitor::EngineRPMMonitor()
	: lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0),
	  enabled(true) {  // Start enabled for testing/debug
    instance = this;
}

void EngineRPMMonitor::begin() {
    pinMode(ENGINE_RPM_SENSOR_PIN, INPUT_PULLUP);

    // Initialize all counters before enabling interrupt
    unsigned long currentTime = millis();
    pulseCount = 0;
    lastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    lastCalculationTime = currentTime;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    // Enable interrupt after initialization
    attachInterrupt(digitalPinToInterrupt(ENGINE_RPM_SENSOR_PIN),
                   handleInterrupt,
                   FALLING);

    Serial.println("EngineRPMMonitor: Initialized on GPIO " + String(ENGINE_RPM_SENSOR_PIN));
}

void EngineRPMMonitor::handleInterrupt() {
    // Only process interrupts if monitoring is enabled
    if (!instance || !instance->enabled) {
        return;
    }

    unsigned long currentTime = millis();

    // Debounce with 10ms minimum between pulses
    if (currentTime - lastPulseTime > 10) {
        pulseCount++;
        lastPulseTime = currentTime;
    }
}

void EngineRPMMonitor::update() {
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
            // No pulses for timeout period - engine likely stopped
            currentRPM = 0.0f;
        }

        lastPulseCountSnapshot = currentPulseCount;
        lastCalculationTime = currentTime;
    }
}

bool EngineRPMMonitor::isReceivingSignal() const {
    // Basic pulse detection - shows any recent interrupt activity (for debug)
    return (millis() - lastPulseTime) < RPM_TIMEOUT_MS;
}

bool EngineRPMMonitor::isValidSignal() const {
    // Filtered signal validation - requires stable RPM to be considered real
    // This filters out electrical noise that creates sporadic low-rate pulses
    return isReceivingSignal() && currentRPM >= MIN_STABLE_RPM;
}

void EngineRPMMonitor::reset() {
    unsigned long currentTime = millis();
    pulseCount = 0;
    lastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;
    lastCalculationTime = currentTime;
}

void EngineRPMMonitor::printStatus() {
    Serial.println("=== EngineRPMMonitor Status ===");
    Serial.println("Current RPM: " + String(currentRPM, 1));
    Serial.println("Total Pulses: " + String(pulseCount));
    Serial.println("Signal Active: " + String(isReceivingSignal() ? "Yes" : "No"));
    Serial.println("Valid Signal: " + String(isValidSignal() ? "Yes" : "No"));
    Serial.println("Last Pulse: " + String(millis() - lastPulseTime) + "ms ago");
    Serial.println("Enabled: " + String(enabled ? "Yes" : "No"));
}

void EngineRPMMonitor::setEnabled(bool enable) {
    enabled = enable;
    if (!enable) {
        // When disabling, reset all counters to prevent stale readings
        reset();
    }
}