#include <Arduino.h>
#include "config.h"
#include "version.h"
#include "classes/SpeedometerWheel.h"
#include "classes/GearIndicator.h"
#include "classes/RPMHandler.h"
#include "classes/DisplayManager.h"
#include "classes/DriveshaftMonitor.h"

GearIndicator gearIndicator;
SpeedometerWheel speedometer;
DisplayManager displayManager;
DriveshaftMonitor driveshaftMonitor;
RPMHandler rpmHandler(&gearIndicator, &speedometer, &driveshaftMonitor);

void setup() {
  Serial.begin(115200);
  Serial.println("=== Mechanical Speedometer Demo ===");
  Serial.print("Version: ");
  Serial.println(VERSION_STRING);
  Serial.println("Starting system initialization...");

  // Initialize display first
  if (!displayManager.begin()) {
    Serial.println("Warning: Display initialization failed, continuing without display");
  }

  gearIndicator.begin();
  speedometer.begin();
  driveshaftMonitor.begin();

  // Test servo output immediately after initialization

  // Demo sequence: calibrate speedometer, then run demo
  delay(2000);

  // Serial.println("Starting continuous stepper test to verify sensor...");
  // displayManager.showCalibrationScreen("Stepper Test");
  // speedometer.continuousStepperTest();

  Serial.println("Calibrating speedometer...");
  displayManager.showCalibrationScreen("Calibrating...");

  if (speedometer.calibrateHome()) {
    Serial.println("Speedometer calibrated successfully!");
    displayManager.showCalibrationScreen("Calibration OK");
    delay(1000);

    // Start real RPM-based speed calculation
    Serial.println("Starting RPM-based speed calculation...");
    Serial.println("Ready to receive driveshaft RPM input for speed calculations");

    // Initial neutral state
    displayManager.updateStatus(NEUTRAL, 0, GEAR_NAMES[NEUTRAL]);
    displayManager.updateDiagnostics(false, false, true);
  } else {
    Serial.println("Speedometer calibration failed!");
    displayManager.showErrorScreen("Calibration Failed");
    delay(3000);
  }
}

unsigned long lastRpmReport = 0;
unsigned long lastStatusUpdate = 0;

void loop() {
  // Update all components for smooth transitions
  gearIndicator.update();
  speedometer.update();
  displayManager.update();
  driveshaftMonitor.update();

  // Update display diagnostics with current component states
  displayManager.updateDiagnostics(
    gearIndicator.isInTransition(),
    speedometer.isInTransition(),
    speedometer.getCalibrationStatus()
  );

  // Get current driveshaft RPM and calculate estimated engine RPM
  unsigned long currentTime = millis();
  float driveshaftRPM = driveshaftMonitor.getRPM();

  // Simulate engine RPM based on driveshaft RPM and estimated gear ratio
  // For now, assume 2nd gear (2.21:1) * differential (3.9:1) = ~8.6:1 overall
  // This gives a reasonable estimate until we add real engine RPM sensing
  float estimatedEngineRPM = 0.0f;
  if (driveshaftRPM > 10.0f) {  // Only calculate if we have meaningful driveshaft RPM
    estimatedEngineRPM = driveshaftRPM * 3.9f * 2.0f;  // Assume average gear ratio
  }

  // Update RPM handler with real driveshaft data and estimated engine RPM
  rpmHandler.update(estimatedEngineRPM);

  // Report RPM and status every 2 seconds
  if (currentTime - lastRpmReport > 2000) {
    lastRpmReport = currentTime;
    Serial.println("Driveshaft: " + String(driveshaftRPM, 1) + " RPM | " +
                   "Engine: " + String(estimatedEngineRPM, 0) + " RPM | " +
                   "Speed: " + String(rpmHandler.getCurrentSpeed()) + " MPH | " +
                   "Gear: " + String(GEAR_NAMES[rpmHandler.getCurrentGear()]) + " | " +
                   "Signal: " + String(driveshaftMonitor.isReceivingSignal() ? "OK" : "NO"));
  }

  // Update display with current RPM handler status every 500ms
  if (currentTime - lastStatusUpdate > 500) {
    lastStatusUpdate = currentTime;
    displayManager.updateStatus(
      rpmHandler.getCurrentGear(),
      rpmHandler.getCurrentSpeed(),
      GEAR_NAMES[rpmHandler.getCurrentGear()]
    );
  }

  delay(10);  // Small delay for smooth animation
}