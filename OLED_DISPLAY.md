# OLED Display Guide

The ESP32 built-in 0.96" OLED display shows real-time system information and status.

## Display Features

### Screen Layout
- **Header**: Project name and version number
- **Content Area**: Dynamic content based on current page
- **Footer**: Page indicator (Page X/3)

### Display Pages

#### Page 1: Status
- Current gear position and name
- Current speed in MPH
- System status indicators:
  - `S` = Servo moving
  - `M` = Stepper motor moving
  - `!` = Not calibrated

#### Page 2: Diagnostics
- Calibration status (YES/NO)
- Servo status (MOVING/IDLE)
- Stepper motor status (MOVING/IDLE)
- Free RAM in bytes

#### Page 3: Settings
- Target vehicle: 1970 MGB
- Differential ratio: 3.9:1
- Tire size: 165-80R13
- System uptime in seconds

## Display Configuration

### Hardware Pins (config.h)
```cpp
#define OLED_SDA 4    // GPIO 4 - SDA pin for I2C
#define OLED_SCL 15   // GPIO 15 - SCL pin for I2C
#define OLED_RST 16   // GPIO 16 - Reset pin for OLED
```

### Display Settings
- **Resolution**: 128x64 pixels
- **Update Rate**: 250ms (4 FPS)
- **I2C Address**: 0x3C
- **Font**: ArialMT_Plain (10pt and 16pt)

## Special Screens

### Boot Screen
- Shows project title and version
- Displays "Initializing..." message
- Automatically shown at startup

### Calibration Screen
- Shows "CALIBRATION" header
- Dynamic status message
- Used during speedometer calibration

### Error Screen
- Shows "ERROR" header
- Displays error message
- Used for calibration failures

## Display Manager API

### Basic Operations
```cpp
DisplayManager displayManager;

// Initialize display
displayManager.begin();

// Update display (call in main loop)
displayManager.update();

// Control brightness (0-255)
displayManager.setBrightness(128);
```

### Status Updates
```cpp
// Update gear and speed info
displayManager.updateStatus(gear, speed, gearName);

// Update system diagnostics
displayManager.updateDiagnostics(servoMoving, stepperMoving, calibrated);
```

### Special Screens
```cpp
// Show boot screen
displayManager.showBootScreen();

// Show calibration progress
displayManager.showCalibrationScreen("Calibrating...");

// Show error message
displayManager.showErrorScreen("Calibration Failed");
```

### Page Navigation
```cpp
// Switch to next page
displayManager.nextPage();

// Switch to previous page
displayManager.previousPage();
```

## Memory Usage

The display system adds approximately:
- **RAM**: ~470 bytes (from 21,892 to 22,364 bytes)
- **Flash**: ~37KB (from 289,941 to 327,241 bytes)

## Display Content Updates

The display automatically refreshes every 250ms with:
- Real-time gear position and speed
- Component movement indicators
- System status and diagnostics
- Memory usage and uptime

## Troubleshooting

### Display Not Working
1. Check I2C wiring (SDA, SCL, RST pins)
2. Verify I2C address (0x3C)
3. Check power connections
4. Monitor serial output for initialization messages

### Display Flickering
1. Ensure stable power supply
2. Check I2C pull-up resistors
3. Verify 250ms update interval

### Content Not Updating
1. Confirm `displayManager.update()` is called in main loop
2. Check status update calls in demo sequence
3. Verify component state reporting