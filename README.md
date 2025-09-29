# ESP32 Mechanical Speedometer

A realistic mechanical speedometer and gear indicator system for classic cars, built with ESP32 and authentic automotive components. Designed specifically for a 1970 MGB with three-speed manual transmission.

## Project Status

✅ **Core System Implemented** (Version 2.0.0+)
- Intelligent gear detection using engine RPM and vehicle speed analysis
- Dual RPM monitoring (engine and driveshaft sensors)
- Smooth servo and stepper motor control with cubic easing
- Complete vehicle physics simulation with authentic 1970 MGB specifications
- Gear stability filtering to prevent oscillation during shifts
- Flexible GPIO configuration for all components

🔧 **Hardware Integration Ready**
- All classes accept GPIO pins in constructors
- Comprehensive testing methods for motor and sensor validation
- Non-blocking architecture for real-time operation
- Built-in diagnostic and debugging capabilities

📊 **Key Metrics**
- **Gear Detection**: 750ms stability timeout with ±0.25 ratio tolerance
- **Servo Transitions**: 800ms smooth cubic easing
- **Stepper Transitions**: 1200ms smooth cubic easing
- **Update Rate**: 10Hz main loop with interrupt-driven RPM sensing
- **Speed Range**: 0-90 MPH with floating-point precision

## Features

- **Intelligent Gear Detection**: Automatically calculates gear based on engine RPM and vehicle speed using actual 1970 MGB transmission ratios
- **Dual RPM Monitoring**: Separate engine and driveshaft RPM sensors for complete drivetrain simulation
- **Smooth Transitions**: Servo and stepper motors use cubic easing for natural movement (800ms servo, 1200ms stepper)
- **Gear Stability Filtering**: 750ms confirmation timeout prevents erratic display during gear shifts
- **Authentic Vehicle Physics**: Uses actual tire diameter, differential ratio, and transmission ratios for realistic calculations
- **Flexible GPIO Configuration**: All classes accept GPIO pins in constructors for hardware flexibility
- **Comprehensive Speed Calculation**: Multiple precision levels (int and float) for different use cases
- **Non-blocking Architecture**: All components use update() methods for responsive real-time operation

## Hardware Components

### Microcontroller
- **ESP32 Development Board** with integrated 0.96" OLED display, CH340 driver, WiFi+BLE

### Motors & Sensors
- **28BYJ-48 Stepper Motor** with ULN2003 driver board (speedometer needle)
- **DM-S0020 Ultra-Micro Servo** (gear position indicator)
- **Optical Endstop Sensors** with 1M cable for speedometer homing and RPM measurement
- **Dual RPM Sensors**: Separate optical sensors for engine and driveshaft RPM monitoring

### Vehicle Specifications (1970 MGB)
- **Transmission Ratios**: R:3.44, 1st:3.44, 2nd:2.21, 3rd:1.37
- **Differential Ratio**: 3.9:1
- **Tire Size**: 165-80R13 (23" diameter)

## Software Architecture

### Core Classes

#### `DriveshaftToMPHHandler`
Central controller that processes engine and driveshaft RPM inputs to determine:
- Current vehicle speed (calculated from driveshaft RPM and tire specs)
- Optimal gear selection (derived from RPM ratios)
- Stability timeouts during gear changes

#### `GearIndicator`
Controls servo-driven gear position display with intelligent detection:
- **Automatic Gear Detection**: Calculates gear from engine RPM and vehicle speed
- **Vehicle Physics Integration**: Uses actual 1970 MGB transmission and differential ratios
- **Gear Stability Logic**: 750ms confirmation prevents oscillation during shifts
- **Smooth 800ms cubic easing transitions**
- **Servo Positions**: Reverse (0°), Neutral (15°), 1st (30°), 2nd (45°), 3rd (60°)
- **Configurable GPIO**: Accepts servo pin in constructor
- Non-blocking update() method for main loop integration

#### `SpeedometerWheel`
Manages stepper motor speedometer with precision control:
- **Optical endstop homing** for precise calibration and zero positioning
- **Smooth 1200ms cubic easing** for realistic speed changes
- **Dual precision methods**: getCurrentMPH() (int) and getCurrentSpeedMPH() (float)
- **Speed range**: 0-90 MPH with 2048 steps per revolution
- **Shortest-path rotation** logic for efficient movement
- **Comprehensive testing**: Built-in motor and sensor diagnostic methods

#### `DriveshaftInterruptHandler` & `EngineRPMInterruptHandler`
Handle RPM measurement from optical sensors:
- **Interrupt-driven**: High-precision RPM calculation using hardware interrupts
- **Configurable GPIO**: Accept sensor pins in constructors
- **Filtering**: Built-in signal conditioning and noise reduction
- **Real-time updates**: Continuous RPM monitoring for responsive gear detection

## Hardware Photos

### CAN Bus Interface View
![CAN Bus Interface](docs/can-view.JPG)
*CAN bus interface components and connections for reading engine RPM data*

### Servo Mechanism View
![Servo Gear Indicator](docs/servo-view.JPG)
*Servo-driven gear position indicator mechanism showing physical implementation*

> **📋 Technical Details**: Complete wiring diagrams and connection tables are available in [docs/wiring-diagram.md](docs/wiring-diagram.md). For Fritzing diagrams and PCB layouts, see [docs/fritzing-parts-list.md](docs/fritzing-parts-list.md).

## Pin Configuration

```cpp
// Servo Control
#define SERVO_PIN 19          // PWM pin for gear indicator servo

// Stepper Motor Control (28BYJ-48 with ULN2003)
#define STEPPER_PIN_1 25      // Stepper motor phase 1 (IN1)
#define STEPPER_PIN_2 26      // Stepper motor phase 2 (IN2)
#define STEPPER_PIN_3 27      // Stepper motor phase 3 (IN3)
#define STEPPER_PIN_4 32      // Stepper motor phase 4 (IN4)

// Optical Sensors
#define ENDSTOP_PIN 5         // Optical endstop for speedometer homing
#define DRIVESHAFT_SENSOR_PIN 12  // Driveshaft RPM optical sensor
#define ENGINE_RPM_SENSOR_PIN 13  // Engine RPM optical sensor

// OLED Display (I2C)
// Uses ESP32 default I2C pins (SDA=21, SCL=22)
// No explicit pin definitions needed - Wire library uses defaults
```

## Usage

### Basic Setup
```cpp
#include "classes/DriveshaftToMPHHandler.h"
#include "classes/GearIndicator.h"
#include "classes/SpeedometerWheel.h"
#include "classes/DriveshaftInterruptHandler.h"
#include "classes/EngineRPMInterruptHandler.h"

// Initialize components with GPIO pins
GearIndicator gearIndicator(SERVO_PIN);
SpeedometerWheel speedometer;
DriveshaftInterruptHandler driveshaftMonitor(DRIVESHAFT_SENSOR_PIN);
EngineRPMInterruptHandler engineRPMMonitor(ENGINE_RPM_SENSOR_PIN);
DriveshaftToMPHHandler mphHandler(&gearIndicator, &speedometer, &driveshaftMonitor);

void setup() {
    Serial.begin(115200);

    // Initialize all components
    gearIndicator.begin();
    speedometer.begin();
    driveshaftMonitor.begin();
    engineRPMMonitor.begin();

    // Enable RPM monitoring
    driveshaftMonitor.setEnabled(true);
    engineRPMMonitor.setEnabled(true);

    // Optional: Calibrate speedometer on startup
    // speedometer.calibrateHome();
}

void loop() {
    // Update RPM monitors
    driveshaftMonitor.update();
    engineRPMMonitor.update();

    // Get current readings
    float driveshaftRPM = driveshaftMonitor.getRPM();
    float engineRPM = engineRPMMonitor.getRPM();

    // Update speed calculation
    mphHandler.update(driveshaftRPM);
    int speedMPH = mphHandler.getCurrentSpeed();

    // Update gear based on engine RPM and speed
    gearIndicator.updateGearFromRPM(engineRPM, speedMPH);

    // Maintain smooth transitions
    gearIndicator.update();
    speedometer.update();

    delay(100);  // 10Hz update rate
}
```

### Manual Control
```cpp
// Direct gear control (bypasses automatic detection)
gearIndicator.setGear(GEAR_2);

// Direct speed control
speedometer.moveToMPH(45);

// Get current readings
float currentSpeedFloat = speedometer.getCurrentSpeedMPH();  // Floating point precision
int currentSpeedInt = speedometer.getCurrentMPH();           // Integer precision
Gear currentGear = gearIndicator.getCurrentGear();

// Check transition status
if (!gearIndicator.isInTransition()) {
    Serial.println("Gear change complete");
}
if (!speedometer.isInTransition()) {
    Serial.println("Speed change complete");
}
```

## Build Instructions

1. **Hardware Setup**: Connect components according to pin configuration
2. **Software**: Upload code using PlatformIO
3. **Calibration**: System automatically calibrates speedometer on first boot
4. **Testing**: Use demo mode or connect to actual vehicle RPM sensors

### PlatformIO Commands
```bash
# Build project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

## Development History

### Commit Log

- **a5d2799** - Add DriveshaftToMPHHandler class with gear stability detection and smooth transitions
  - Created DriveshaftToMPHHandler class with 1970 MGB transmission specifications
  - Added gear stability timeout logic (750ms confirmation period)
  - Implemented smooth easing transitions for both gear indicator and speedometer
  - Updated constructor formatting to tab-indented one-per-line style
  - Added intelligent gear detection that handles shifting scenarios

- **8edfc03** - Update Claude Code settings
  - Updated development environment configuration

- **5a6fd23** - Initial commit: ESP32 mechanical speedometer project
  - ESP32 with integrated OLED display support
  - SpeedometerWheel class for stepper motor control with optical endstop
  - GearIndicator class for servo-controlled gear display
  - Configuration for 28BYJ-48 stepper motor and DM-S0020 servo
  - GPIO pin definitions and gear enum system
  - PlatformIO configuration with all required libraries

## Technical Details

### Gear Detection Logic
The system determines gear selection by:
1. Calculating actual transmission ratio from engine/driveshaft RPM
2. Comparing against known MGB transmission ratios
3. Requiring 750ms stability before confirming gear changes
4. Defaulting to NEUTRAL during shifts or when ratios don't match

### Smooth Movement System
Both motors use cubic easing curves:
- **Acceleration phase**: Gradual speed increase
- **Constant phase**: Steady movement
- **Deceleration phase**: Smooth approach to target

This creates natural, realistic movement that mimics actual mechanical gauges.

## License

This project is designed for educational and hobbyist use with classic automotive restoration projects.

## Contributing

Contributions welcome! Please follow the established code style guidelines in CLAUDE.md.