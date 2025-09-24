#include <Arduino.h>
#include "config.h"
#include "version.h"
#include "classes/SpeedometerWheel.h"
#include "classes/GearIndicator.h"
#include "classes/RPMHandler.h"
#include "classes/DisplayManager.h"

GearIndicator gearIndicator;
SpeedometerWheel speedometer;
DisplayManager displayManager;

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

    // Start demo sequence
    Serial.println("Starting demo sequence...");

    gearIndicator.setGear(GEAR_1);
    speedometer.moveToMPH(15);

    // Update display with initial status
    displayManager.updateStatus(GEAR_1, 15, GEAR_NAMES[GEAR_1]);
    displayManager.updateDiagnostics(false, false, true);

    // More transitions will happen in loop
  } else {
    Serial.println("Speedometer calibration failed!");
    displayManager.showErrorScreen("Calibration Failed");
    delay(3000);
  }
}

unsigned long lastTransition = 0;
int demoStep = 0;

void loop() {
  // Update all components for smooth transitions
  gearIndicator.update();
  speedometer.update();
  displayManager.update();

  // Update display diagnostics with current component states
  displayManager.updateDiagnostics(
    gearIndicator.isInTransition(),
    speedometer.isInTransition(),
    speedometer.getCalibrationStatus()
  );

  // Demo sequence with timed transitions
  unsigned long currentTime = millis();
  if (currentTime - lastTransition > 4000) {  // Every 4 seconds
    lastTransition = currentTime;

    Gear newGear = GEAR_1;  // Default initialization
    int newSpeed = 0;       // Default initialization

    switch(demoStep) {
      case 0:
        newGear = GEAR_2;
        newSpeed = 35;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
      case 1:
        newGear = GEAR_3;
        newSpeed = 55;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
      case 2:
        newGear = GEAR_2;
        newSpeed = 25;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
      case 3:
        newGear = NEUTRAL;
        newSpeed = 0;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
      case 4:
        newGear = REVERSE;
        newSpeed = 5;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
      default:
        // Reset demo
        demoStep = -1;
        newGear = GEAR_1;
        newSpeed = 15;
        gearIndicator.setGear(newGear);
        speedometer.moveToMPH(newSpeed);
        break;
    }

    // Update display with new status
    displayManager.updateStatus(newGear, newSpeed, GEAR_NAMES[newGear]);

    demoStep++;
  }

  delay(10);  // Small delay for smooth animation
}