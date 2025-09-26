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

  // Enable driveshaft monitor for testing - shows noise vs real signals
  driveshaftMonitor.setEnabled(true);
  Serial.println("DriveshaftMonitor enabled - will show noise until real sensor connected");

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
unsigned long lastDemoTransition = 0;
int demoStep = 0;
bool demoMode = true;  // Enable demo mode when no driveshaft signal

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

  // Check if we should use RPM handler or demo mode
  // Use isValidSignal() for control to filter noise, but keep isReceivingSignal() for debug
  if (driveshaftMonitor.isEnabled() && driveshaftMonitor.isValidSignal() && driveshaftRPM > 10.0f) {
    // Real RPM mode - use driveshaft sensor data
    if (demoMode) {
      Serial.println("Driveshaft signal detected - switching to RPM mode");
      demoMode = false;
    }
    rpmHandler.update(estimatedEngineRPM);
  } else {
    // Demo mode - run original demo sequence when no driveshaft signal
    if (!demoMode) {
      Serial.println("No driveshaft signal - switching to demo mode");
      demoMode = true;
      lastDemoTransition = currentTime;  // Reset demo timing
    }

    // Run demo sequence
    if (currentTime - lastDemoTransition > 4000) {  // Every 4 seconds
      lastDemoTransition = currentTime;

      Gear newGear = NEUTRAL;
      int newSpeed = 0;

      switch(demoStep) {
        case 0:
          newGear = GEAR_1; newSpeed = 15;
          break;
        case 1:
          newGear = GEAR_2; newSpeed = 35;
          break;
        case 2:
          newGear = GEAR_3; newSpeed = 55;
          break;
        case 3:
          newGear = GEAR_2; newSpeed = 25;
          break;
        case 4:
          newGear = NEUTRAL; newSpeed = 0;
          break;
        default:
          demoStep = -1;
          newGear = GEAR_1; newSpeed = 15;
          break;
      }

      gearIndicator.setGear(newGear);
      speedometer.moveToMPH(newSpeed);
      displayManager.updateStatus(newGear, newSpeed, GEAR_NAMES[newGear]);
      demoStep++;
    }
  }

  // Report RPM and status every 2 seconds
  if (currentTime - lastRpmReport > 2000) {
    lastRpmReport = currentTime;
    Serial.println("Driveshaft: " + String(driveshaftRPM, 1) + " RPM | " +
                   "Engine: " + String(estimatedEngineRPM, 0) + " RPM | " +
                   "Speed: " + String(rpmHandler.getCurrentSpeed()) + " MPH | " +
                   "Gear: " + String(GEAR_NAMES[rpmHandler.getCurrentGear()]) + " | " +
                   "Signal: " + String(driveshaftMonitor.isReceivingSignal() ? "OK" : "NO"));
  }

  // Update display with current status every 500ms
  if (currentTime - lastStatusUpdate > 500) {
    lastStatusUpdate = currentTime;
    if (!demoMode) {
      // RPM mode - show RPM handler data
      displayManager.updateStatus(
        rpmHandler.getCurrentGear(),
        rpmHandler.getCurrentSpeed(),
        GEAR_NAMES[rpmHandler.getCurrentGear()]
      );
    }
    // Note: In demo mode, display is updated immediately when demo transitions occur
  }

  delay(10);  // Small delay for smooth animation
}