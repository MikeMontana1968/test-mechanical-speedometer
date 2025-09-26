#include "RPMHandler.h"
#include <Arduino.h>

// 1970 MGB Three-speed manual transmission ratios
const float RPMHandler::TRANSMISSION_RATIOS[5] = {
    3.44f,  // Reverse gear
    3.44f,  // 1st gear
    2.21f,  // 2nd gear
    1.37f,  // 3rd gear
    1.0f    // Not used (placeholder)
};

RPMHandler::RPMHandler(GearIndicator* gearInd, SpeedometerWheel* speedo, DriveshaftMonitor* driveshaft)
	: gearIndicator(gearInd),
	  speedometer(speedo),
	  driveshaftMonitor(driveshaft),
	  currentGear(NEUTRAL),
	  candidateGear(NEUTRAL),
	  currentSpeed(0),
	  lastEngineRPM(0.0f),
	  lastDriveshaftRPM(0.0f),
	  lastValidGearTime(0),
	  candidateGearStartTime(0) {
}

void RPMHandler::update(float engineRPM, float driveshaftRPM) {
    lastEngineRPM = engineRPM;
    lastDriveshaftRPM = driveshaftRPM;
    unsigned long currentTime = millis();

    // Calculate speed from driveshaft RPM
    int newSpeed = calculateSpeedFromDriveshaftRPM(driveshaftRPM);

    // Detect potential gear based on RPM ratios
    Gear detectedGear = calculateOptimalGear(engineRPM, driveshaftRPM);

    // Evaluate gear stability with timing logic
    Gear confirmedGear = evaluateGearStability(detectedGear, currentTime);

    // Update speed if changed significantly (avoid micro-adjustments)
    if (abs(newSpeed - currentSpeed) > 1) {
        currentSpeed = newSpeed;
        if (speedometer) {
            speedometer->moveToMPH(currentSpeed);
        }
    }

    // Update gear if confirmed gear changed
    if (confirmedGear != currentGear) {
        currentGear = confirmedGear;
        if (gearIndicator) {
            gearIndicator->setGear(currentGear);
        }

        Serial.print("Gear confirmed: ");
        Serial.print(GEAR_NAMES[currentGear]);
        Serial.print(" at ");
        Serial.print(currentSpeed);
        Serial.print(" MPH (Engine: ");
        Serial.print(engineRPM);
        Serial.print(" RPM, Driveshaft: ");
        Serial.print(driveshaftRPM);
        Serial.println(" RPM)");
    }
}

void RPMHandler::update(float engineRPM) {
    // Use DriveshaftMonitor for automatic driveshaft RPM reading
    if (driveshaftMonitor) {
        float driveshaftRPM = driveshaftMonitor->getRPM();
        update(engineRPM, driveshaftRPM);
    } else {
        // Fallback: use last known driveshaft RPM
        update(engineRPM, lastDriveshaftRPM);
    }
}

Gear RPMHandler::calculateOptimalGear(float engineRPM, float driveshaftRPM) {
    // Handle special cases - low RPM indicates neutral or stopped
    if (engineRPM < 100.0f || abs(driveshaftRPM) < 10.0f) {
        return NEUTRAL;
    }

    // Calculate actual transmission ratio from RPM readings
    float actualRatio = engineRPM / (abs(driveshaftRPM) * DIFFERENTIAL_RATIO);

    // Check reverse gear (if driveshaft is negative)
    if (driveshaftRPM < 0 && isGearRatioValid(actualRatio, REVERSE)) {
        return REVERSE;
    }

    // Check forward gears 1-3
    for (int i = GEAR_1; i <= GEAR_3; i++) {
        if (isGearRatioValid(actualRatio, static_cast<Gear>(i))) {
            return static_cast<Gear>(i);
        }
    }

    // No valid gear ratio found - likely shifting or clutch disengaged
    return NEUTRAL;
}

Gear RPMHandler::evaluateGearStability(Gear detectedGear, unsigned long currentTime) {
    // If detected gear matches candidate, continue timing
    if (detectedGear == candidateGear) {
        // Check if gear has been stable long enough
        if (currentTime - candidateGearStartTime >= GEAR_STABILITY_TIMEOUT_MS) {
            lastValidGearTime = currentTime;
            return candidateGear;  // Confirm this gear
        }
        // Still timing this candidate gear
        return currentGear;  // Keep current gear until confirmed
    }

    // New gear detected - start timing
    candidateGear = detectedGear;
    candidateGearStartTime = currentTime;

    // If we haven't had a valid gear for too long, default to neutral
    if (currentTime - lastValidGearTime > GEAR_STABILITY_TIMEOUT_MS) {
        return NEUTRAL;
    }

    // Keep current gear until new one is confirmed
    return currentGear;
}

bool RPMHandler::isGearRatioValid(float actualRatio, Gear gear) {
    if (gear < REVERSE || gear > GEAR_3) {
        return false;
    }

    float expectedRatio = TRANSMISSION_RATIOS[gear];
    float difference = abs(actualRatio - expectedRatio);

    return difference <= GEAR_RATIO_TOLERANCE;
}

int RPMHandler::calculateSpeedFromDriveshaftRPM(float driveshaftRPM) {
    if (driveshaftRPM <= 0) {
        return 0;
    }

    // Calculate wheel RPM from driveshaft RPM
    float wheelRPM = driveshaftRPM / DIFFERENTIAL_RATIO;

    // Calculate tire circumference in inches
    float tireCircumference = PI * TIRE_DIAMETER_INCHES;

    // Calculate speed in MPH
    // (wheel RPM) * (circumference in inches) * (minutes/hour) / (inches/mile)
    float speedMPH = (wheelRPM * tireCircumference * MINUTES_PER_HOUR) / INCHES_PER_MILE;

    return (int)round(speedMPH);
}

float RPMHandler::calculateExpectedEngineRPM(Gear gear, float driveshaftRPM) {
    if (gear == NEUTRAL || driveshaftRPM <= 0) {
        return 0.0f;
    }

    return driveshaftRPM * DIFFERENTIAL_RATIO * TRANSMISSION_RATIOS[gear];
}

float RPMHandler::getTransmissionRatio(Gear gear) const {
    if (gear >= REVERSE && gear <= GEAR_3) {
        return TRANSMISSION_RATIOS[gear];
    }
    return 1.0f;  // Default for invalid gear
}

void RPMHandler::printStatus() {
    Serial.println("=== RPM Handler Status ===");
    Serial.print("Current Gear: ");
    Serial.println(GEAR_NAMES[currentGear]);
    Serial.print("Current Speed: ");
    Serial.print(currentSpeed);
    Serial.println(" MPH");
    Serial.print("Engine RPM: ");
    Serial.println(lastEngineRPM);
    Serial.print("Driveshaft RPM: ");
    Serial.println(lastDriveshaftRPM);
    Serial.print("Differential Ratio: ");
    Serial.println(DIFFERENTIAL_RATIO);
    Serial.print("Tire Diameter: ");
    Serial.print(TIRE_DIAMETER_INCHES);
    Serial.println(" inches");

    Serial.println("Transmission Ratios:");
    Serial.print("  Reverse: ");
    Serial.println(TRANSMISSION_RATIOS[REVERSE]);
    Serial.print("  1st: ");
    Serial.println(TRANSMISSION_RATIOS[GEAR_1]);
    Serial.print("  2nd: ");
    Serial.println(TRANSMISSION_RATIOS[GEAR_2]);
    Serial.print("  3rd: ");
    Serial.println(TRANSMISSION_RATIOS[GEAR_3]);
    Serial.println("========================");
}