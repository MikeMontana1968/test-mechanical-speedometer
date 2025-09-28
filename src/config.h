#ifndef CONFIG_H
#define CONFIG_H

// GPIO Pin Definitions
#define SERVO_PIN 19  // GPIO 19 - PWM capable pin for servo control
#define DRIVESHAFT_SENSOR_PIN 18  // GPIO 18 - Driveshaft optical endstop sensor
#define ENGINE_RPM_SENSOR_PIN 17  // GPIO 17 - Engine RPM optical sensor

// OLED Display Settings - using default I2C pins like working project
// Default I2C pins: SDA=21, SCL=22 (ESP32 defaults)
// No explicit pin definitions needed - Wire library uses defaults

// Gear Selection Definitions
enum Gear {
    REVERSE = 0,    // 0 degrees
    NEUTRAL = 1,    // 15 degrees
    GEAR_1 = 2,     // 30 degrees
    GEAR_2 = 3,     // 45 degrees
    GEAR_3 = 4      // 60 degrees
};

// Gear angle lookup (in degrees)
extern const int GEAR_ANGLES[5];

// Gear string lookup
extern const char* GEAR_NAMES[5];

// Speedometer Stepper Motor Definitions (moved to avoid I2C conflict)
#define STEPPER_PIN_1 25  // GPIO 25 - Stepper motor pin 1
#define STEPPER_PIN_2 26  // GPIO 26 - Stepper motor pin 2
#define STEPPER_PIN_3 27  // GPIO 27 - Stepper motor pin 3
#define STEPPER_PIN_4 32  // GPIO 32 - Stepper motor pin 4
#define ENDSTOP_PIN 5     // GPIO 5 - Optical endstop sensor

// Speedometer Range Configuration
#define MIN_SPEED_MPH 0
#define MAX_SPEED_MPH 90
#define SPEED_RANGE (MAX_SPEED_MPH - MIN_SPEED_MPH)

// 28BYJ-48 Stepper Motor Specifications
#define STEPS_PER_REVOLUTION 2048  // Steps per full revolution for 28BYJ-48
#define STEPPER_RPM 15            // Maximum recommended RPM

// Calculate steps per MPH (assuming full revolution covers full speed range)
#define STEPS_PER_MPH (STEPS_PER_REVOLUTION / SPEED_RANGE)

#endif // CONFIG_H