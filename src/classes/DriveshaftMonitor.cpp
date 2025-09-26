#include "DriveshaftMonitor.h"
#include <Arduino.h>

volatile unsigned long DriveshaftMonitor::pulseCount = 0;
volatile unsigned long DriveshaftMonitor::lastPulseTime = 0;
DriveshaftMonitor* DriveshaftMonitor::instance = nullptr;

DriveshaftMonitor::DriveshaftMonitor()
	: lastCalculationTime(0),
	  currentRPM(0.0f),
	  lastPulseCountSnapshot(0) {
    instance = this;
}

void DriveshaftMonitor::begin() {
    pinMode(DRIVESHAFT_SENSOR_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(DRIVESHAFT_SENSOR_PIN),
                   handleInterrupt,
                   FALLING);

    pulseCount = 0;
    lastPulseTime = 0;
    lastCalculationTime = millis();
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;

    Serial.println("DriveshaftMonitor: Initialized on GPIO " + String(DRIVESHAFT_SENSOR_PIN));
}

void DriveshaftMonitor::handleInterrupt() {
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
        unsigned long pulsesInInterval = currentPulseCount - lastPulseCountSnapshot;
        unsigned long actualInterval = currentTime - lastCalculationTime;

        if (pulsesInInterval > 0 && actualInterval > 0) {
            float pulsesPerMinute = (float)pulsesInInterval * (60000.0f / (float)actualInterval);
            currentRPM = pulsesPerMinute;

            if (currentRPM < MIN_RPM_THRESHOLD) {
                currentRPM = 0.0f;
            }
        } else if (currentTime - lastPulseTime > RPM_TIMEOUT_MS) {
            currentRPM = 0.0f;
        }

        lastPulseCountSnapshot = currentPulseCount;
        lastCalculationTime = currentTime;
    }
}

bool DriveshaftMonitor::isReceivingSignal() const {
    return (millis() - lastPulseTime) < RPM_TIMEOUT_MS;
}

void DriveshaftMonitor::reset() {
    pulseCount = 0;
    lastPulseTime = 0;
    currentRPM = 0.0f;
    lastPulseCountSnapshot = 0;
    lastCalculationTime = millis();
}

void DriveshaftMonitor::printStatus() {
    Serial.println("=== DriveshaftMonitor Status ===");
    Serial.println("Current RPM: " + String(currentRPM, 1));
    Serial.println("Total Pulses: " + String(pulseCount));
    Serial.println("Signal Active: " + String(isReceivingSignal() ? "Yes" : "No"));
    Serial.println("Last Pulse: " + String(millis() - lastPulseTime) + "ms ago");
}