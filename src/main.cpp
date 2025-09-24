#include <Arduino.h>
#include "config.h"
#include "classes/SpeedometerWheel.h"
#include "classes/GearIndicator.h"
#include "classes/RPMHandler.h"

GearIndicator gearIndicator;
SpeedometerWheel speedometer;

void setup() {
  Serial.begin(115200);
  Serial.println("Mechanical Speedometer Demo Starting...");

  gearIndicator.begin();
  speedometer.begin();

  // Demo sequence: calibrate speedometer, then run demo
  delay(2000);

  Serial.println("Calibrating speedometer...");
  if (speedometer.calibrateHome()) {
    Serial.println("Speedometer calibrated successfully!");

    // Start demo sequence
    delay(1000);
    Serial.println("Starting demo sequence...");

    gearIndicator.setGear(GEAR_1);
    speedometer.moveToMPH(15);

    // More transitions will happen in loop
  } else {
    Serial.println("Speedometer calibration failed!");
  }
}

unsigned long lastTransition = 0;
int demoStep = 0;

void loop() {
  // Update both components for smooth transitions
  gearIndicator.update();
  speedometer.update();

  // Demo sequence with timed transitions
  unsigned long currentTime = millis();
  if (currentTime - lastTransition > 4000) {  // Every 4 seconds
    lastTransition = currentTime;

    switch(demoStep) {
      case 0:
        gearIndicator.setGear(GEAR_2);
        speedometer.moveToMPH(35);
        break;
      case 1:
        gearIndicator.setGear(GEAR_3);
        speedometer.moveToMPH(55);
        break;
      case 2:
        gearIndicator.setGear(GEAR_2);
        speedometer.moveToMPH(25);
        break;
      case 3:
        gearIndicator.setGear(NEUTRAL);
        speedometer.moveToMPH(0);
        break;
      case 4:
        gearIndicator.setGear(REVERSE);
        speedometer.moveToMPH(5);
        break;
      default:
        // Reset demo
        demoStep = -1;
        gearIndicator.setGear(GEAR_1);
        speedometer.moveToMPH(15);
        break;
    }

    demoStep++;
  }

  delay(10);  // Small delay for smooth animation
}