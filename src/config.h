#ifndef CONFIG_H
#define CONFIG_H

// GPIO Pin Definitions
#define SERVO_PIN 18  // GPIO 18 - PWM capable pin for servo control

// OLED Display Settings (typical for ESP32 with integrated OLED)
#define OLED_SDA 4    // GPIO 4 - SDA pin for I2C
#define OLED_SCL 15   // GPIO 15 - SCL pin for I2C
#define OLED_RST 16   // GPIO 16 - Reset pin for OLED

// Gear Selection Definitions
enum Gear {
    REVERSE = 0,    // 0 degrees
    NEUTRAL = 1,    // 15 degrees
    GEAR_1 = 2,     // 30 degrees
    GEAR_2 = 3,     // 45 degrees
    GEAR_3 = 4      // 60 degrees
};

// Gear angle lookup (in degrees)
const int GEAR_ANGLES[] = {0, 15, 30, 45, 60};

// Gear string lookup
const char* GEAR_NAMES[] = {"Reverse", "Neutral", "1", "2", "3"};

// Speedometer Stepper Motor Definitions
#define STEPPER_PIN_1 19  // GPIO 19 - Stepper motor pin 1
#define STEPPER_PIN_2 21  // GPIO 21 - Stepper motor pin 2
#define STEPPER_PIN_3 22  // GPIO 22 - Stepper motor pin 3
#define STEPPER_PIN_4 23  // GPIO 23 - Stepper motor pin 4
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