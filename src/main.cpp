#include <Arduino.h>
#include "config.h"
#include "version.h"
#include "classes/SpeedometerWheel.h"
#include "classes/GearIndicator.h"
#include "classes/DriveshaftRPMHandler.h"
#include "classes/DisplayManager.h"
#include "classes/DriveshaftMonitor.h"

GearIndicator gearIndicator;
SpeedometerWheel speedometer;
DisplayManager displayManager;
DriveshaftMonitor driveshaftMonitor;
DriveshaftRPMHandler rpmHandler(&gearIndicator, &speedometer, &driveshaftMonitor);

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

  // Enable driveshaft monitor for testing
  driveshaftMonitor.setEnabled(true);

  // Test servo output immediately after initialization

  // Demo sequence: calibrate speedometer, then run demo
  delay(2000);

  Serial.println("Starting continuous stepper test to verify sensor...");
  displayManager.showCalibrationScreen("Stepper Test");
  speedometer.continuousStepperTest();

  // Serial.println("Calibrating speedometer...");
  // displayManager.showCalibrationScreen("Calibrating...");

  /*
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
  */
}

unsigned long lastRpmReport = 0;
unsigned long lastStatusUpdate = 0;
unsigned long lastDemoTransition = 0;
int demoStep = 0;
bool demoMode = true;  // Enable demo mode when no driveshaft signal

void loop() {
  // Minimal loop during continuous stepper test
  // The continuous test runs in setup() and handles its own loop
  delay(1000);
}