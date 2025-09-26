#include "DriveshaftMonitor.h"
#include <Arduino.h>

volatile unsigned long DriveshaftMonitor::pulseCount = 0;
volatile unsigned long DriveshaftMonitor::lastPulseTime = 0;
DriveshaftMonitor* DriveshaftMonitor::instance = nullptr;

DriveshaftMonitor::DriveshaftMonitor()
	: lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0),
	  enabled(false) {  // Start disabled until explicitly enabled
    instance = this;
}

void DriveshaftMonitor::begin() {
    pinMode(DRIVESHAFT_SENSOR_PIN, INPUT_PULLUP);

    // Initialize all counters before enabling interrupt
    unsigned long currentTime = millis();
    pulseCount = 0;
    lastPulseTime = currentTime;  // Initialize to current time to prevent false triggers
    lastCalculationTime = currentTime;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    // Enable interrupt after initialization
    attachInterrupt(digitalPinToInterrupt(DRIVESHAFT_SENSOR_PIN),
                   handleInterrupt,
                   FALLING);

    Serial.println("DriveshaftMonitor: Initialized on GPIO " + String(DRIVESHAFT_SENSOR_PIN));
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

        // Debug: Log interrupt activity (remove this in production)
        static unsigned long lastDebugPrint = 0;
        if (currentTime - lastDebugPrint > 500) {  // Print max every 500ms
            Serial.println("DriveshaftMonitor: Interrupt triggered (total: " + String(pulseCount) + ")");
            lastDebugPrint = currentTime;
        }
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
            Serial.println("DriveshaftMonitor: Pulse counter reset detected");
        }

        if (pulsesInInterval > 0 && actualInterval > 0) {
            float pulsesPerMinute = (float)pulsesInInterval * (60000.0f / (float)actualInterval);

            // Apply bounds checking
            if (pulsesPerMinute < MIN_RPM_THRESHOLD) {
                currentRPM = 0.0f;
            } else if (pulsesPerMinute > MAX_RPM_THRESHOLD) {
                // Unrealistic RPM - likely calculation error, keep previous value
                Serial.println("DriveshaftMonitor: Unrealistic RPM calculated (" + String(pulsesPerMinute, 0) + "), ignoring");
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
    // Only consider signal valid if we have recent pulses AND a reasonable RPM
    // This filters out electrical noise that creates sporadic low-rate pulses
    return (millis() - lastPulseTime) < RPM_TIMEOUT_MS && currentRPM >= MIN_STABLE_RPM;
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
    Serial.println("Last Pulse: " + String(millis() - lastPulseTime) + "ms ago");
    Serial.println("Enabled: " + String(enabled ? "Yes" : "No"));
}

void DriveshaftMonitor::setEnabled(bool enable) {
    enabled = enable;
    if (!enable) {
        // When disabling, reset all counters to prevent stale readings
        reset();
        Serial.println("DriveshaftMonitor: Disabled");
    } else {
        Serial.println("DriveshaftMonitor: Enabled");
    }
}