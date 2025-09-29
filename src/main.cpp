#include <Arduino.h>
#include "config.h"
#include "version.h"
#include "classes/SpeedometerWheel.h"
#include "classes/GearIndicator.h"
#include "classes/DriveshaftToMPHHandler.h"
#include "classes/DisplayManager.h"
#include "classes/DriveshaftInterruptHandler.h"
#include "classes/EngineRPMInterruptHandler.h"

GearIndicator gearIndicator(SERVO_PIN);
SpeedometerWheel speedometer;
DisplayManager displayManager;
DriveshaftInterruptHandler driveshaftMonitor(DRIVESHAFT_SENSOR_PIN);
EngineRPMInterruptHandler engineRPMMonitor(ENGINE_RPM_SENSOR_PIN);
DriveshaftToMPHHandler mphHandler(&gearIndicator, &speedometer, &driveshaftMonitor);

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
  engineRPMMonitor.begin();

  // Enable monitors for testing
  driveshaftMonitor.setEnabled(true);
  engineRPMMonitor.setEnabled(true);

  // Serial.println("Starting continuous stepper test to verify sensor...");
  // displayManager.showCalibrationScreen("Stepper Test");
  // speedometer.continuousStepperTest();

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
  unsigned long currentTime = millis();

  // Update both RPM monitors
  driveshaftMonitor.update();
  engineRPMMonitor.update();

  // Debug output every 1 second
  if (currentTime - lastRpmReport >= 1000) {
    lastRpmReport = currentTime;

    float driveshaftRPM = driveshaftMonitor.getRPM();
    float engineRPM = engineRPMMonitor.getRPM();

    mphHandler.update(driveshaftRPM);
    engineRPMMonitor.update();
    // Print debug information
    int speedMPH = mphHandler.getCurrentSpeed();

    // Update gear based on engine RPM and speed
    gearIndicator.updateGearFromRPM(engineRPM, speedMPH);
    gearIndicator.update();  // Handle servo smooth transitions
    Gear currentGear = gearIndicator.getCurrentGear();

    Serial.print("Driveshaft: ");
    Serial.print(driveshaftRPM, 0);
    Serial.print("rpm ");
    Serial.print(speedMPH, 0);  
    Serial.print("mph\tEngine: ");
    Serial.print(engineRPM, 0);
    Serial.print("rpm, \tRatio: ");
    Serial.print(engineRPM/driveshaftRPM, 2);
    Serial.print(":1 - \tGear: ");    
    Serial.print(GEAR_NAMES[currentGear]);
    Serial.println();
  }

  delay(100);
}