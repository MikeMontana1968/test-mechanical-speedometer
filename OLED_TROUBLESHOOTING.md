# OLED Display Troubleshooting Guide

## Issue Description
OLED display (integrated 0.96" SSD1306) is not initializing properly during ESP32 startup.

## Current Configuration
- **Display**: 0.96" OLED SSD1306 (128x64 pixels)
- **I2C Address**: 0x3C (trying 0x3D as fallback)
- **SDA Pin**: GPIO 4
- **SCL Pin**: GPIO 15
- **Reset Pin**: GPIO 16
- **Library**: ESP8266 and ESP32 OLED driver for SSD1306 displays

## Debugging Features Added

### 1. Enhanced Debug Output
The system now provides detailed I2C debugging:
```
Initializing OLED display...
I2C SDA pin: GPIO 4
I2C SCL pin: GPIO 15
Reset pin: GPIO 16
I2C initialized with custom pins
Scanning I2C bus for devices...
```

### 2. I2C Device Scanner
Automatically scans all I2C addresses (0x01 to 0x7F) to detect connected devices and specifically identifies OLED displays at common addresses (0x3C, 0x3D).

### 3. Multiple Address Testing
Attempts initialization with both 0x3C and 0x3D addresses to handle different OLED variants.

## Common Issues and Solutions

### Issue 1: Wrong I2C Pins for Your ESP32 Board

**Problem**: The ESP32 board you're using may have different I2C pin assignments.

**Solutions**:
1. **Check your ESP32 board documentation** for correct I2C pins
2. **Try standard ESP32 I2C pins**:
   - SDA: GPIO 21
   - SCL: GPIO 22

**Fix**: Update `src/config.h`:
```cpp
// Try standard ESP32 I2C pins
#define OLED_SDA 21   // GPIO 21 - Standard SDA pin
#define OLED_SCL 22   // GPIO 22 - Standard SCL pin
#define OLED_RST 16   // GPIO 16 - Reset pin
```

**Note**: This will conflict with stepper motor pins. If using stepper, try:
```cpp
// Alternative pins (avoid stepper conflicts)
#define OLED_SDA 2    // GPIO 2 - Alternative SDA
#define OLED_SCL 0    // GPIO 0 - Alternative SCL (be careful - used for boot)
#define OLED_RST 16   // GPIO 16 - Reset pin
```

### Issue 2: Built-in OLED vs External OLED

**Integrated OLED Board**: If your ESP32 has a built-in OLED, the pins are pre-wired and you need to match the board's configuration.

**Common integrated OLED pin configurations**:
```cpp
// TTGO T-Display style boards
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

// Heltec WiFi Kit 32 style boards
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

// Some WEMOS D1 style boards
#define OLED_SDA 5
#define OLED_SCL 4
#define OLED_RST 16
```

### Issue 3: Library Compatibility

**Problem**: Wrong OLED library or incorrect initialization.

**Try Alternative Library Setup**:
1. **Switch to Adafruit Library**:
   Update `platformio.ini`:
   ```ini
   lib_deps =
       adafruit/Adafruit SSD1306@^2.5.7
       adafruit/Adafruit GFX Library@^1.11.5
   ```

2. **Use Different Initialization**:
   ```cpp
   #include <Adafruit_SSD1306.h>
   #include <Adafruit_GFX.h>

   #define SCREEN_WIDTH 128
   #define SCREEN_HEIGHT 64
   Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

   // In begin():
   Wire.begin(OLED_SDA, OLED_SCL);
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
       Serial.println("SSD1306 allocation failed");
       return false;
   }
   ```

### Issue 4: Power Supply Issues

**Check Power Connections**:
- **VCC**: Should be connected to 3.3V (most common) or 5V
- **GND**: Must be connected to ESP32 ground
- **Sufficient Current**: OLED needs ~20mA

**Test**: Try powering OLED from external 3.3V source temporarily.

### Issue 5: Hardware Defects

**OLED Module Dead**: If I2C scan shows no devices at all, the OLED may be damaged.

**Connection Issues**:
- Loose wires (like you had with the servo)
- Cold solder joints
- Damaged traces on PCB

## Diagnostic Steps

### Step 1: Check Serial Output
Look for these debug messages:
```
Initializing OLED display...
I2C SDA pin: GPIO 4
I2C SCL pin: GPIO 15
Reset pin: GPIO 16
I2C initialized with custom pins
Scanning I2C bus for devices...
I2C device found at address 0x3C
  ^ This appears to be an OLED display!
Found 1 I2C device(s)
Attempting to initialize OLED display at address 0x3C...
OLED display initialized successfully!
```

### Step 2: I2C Scanner Results
- **No devices found**: Check wiring, power, pin assignments
- **Device found at 0x3C or 0x3D**: OLED detected, library issue
- **Device found at different address**: Update code with correct address

### Step 3: Try Different Pin Configurations

**Test 1 - Standard ESP32 pins**:
```cpp
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
```

**Test 2 - Alternative pins**:
```cpp
#define OLED_SDA 5
#define OLED_SCL 4
#define OLED_RST 16
```

**Test 3 - Some board variants**:
```cpp
#define OLED_SDA 14
#define OLED_SCL 13
#define OLED_RST 16
```

### Step 4: Hardware Isolation Test

**Simple I2C Test**:
```cpp
void setup() {
    Serial.begin(115200);
    Wire.begin(4, 15);  // Your current pins
    Serial.println("I2C Scanner");
}

void loop() {
    for(byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if(error == 0) {
            Serial.print("Device found at address 0x");
            Serial.println(address, HEX);
        }
    }
    delay(5000);
}
```

## Quick Fixes to Try

### Fix 1: Update Pin Configuration
If you know your board's OLED pins, update `src/config.h`:
```cpp
// Update these based on your ESP32 board
#define OLED_SDA 21   // Change to correct SDA pin
#define OLED_SCL 22   // Change to correct SCL pin
#define OLED_RST 16   // Reset pin (usually correct)
```

### Fix 2: Try 0x3D Address
If scanner finds device at 0x3D, update DisplayManager constructor:
```cpp
DisplayManager::DisplayManager()
    : display(0x3d, OLED_SDA, OLED_SCL),  // Change from 0x3c to 0x3d
```

### Fix 3: Disable OLED Temporarily
To test rest of system, comment out display calls in `main.cpp`:
```cpp
// displayManager.begin();  // Comment out for testing
```

## Expected Working Output
When working correctly, you should see:
```
=== Mechanical Speedometer Demo ===
Version: 1.0.0
Starting system initialization...
Initializing OLED display...
I2C SDA pin: GPIO 4
I2C SCL pin: GPIO 15
Reset pin: GPIO 16
I2C initialized with custom pins
Scanning I2C bus for devices...
I2C device found at address 0x3C
  ^ This appears to be an OLED display!
Found 1 I2C device(s)
Attempting to initialize OLED display at address 0x3C...
OLED display initialized successfully!
```

## Next Steps
1. **Check what your serial monitor shows** during OLED initialization
2. **Note any I2C devices found** and their addresses
3. **Try different pin configurations** if no devices found
4. **Check physical connections** (especially after servo wire issue)

The enhanced debug output will help identify exactly where the initialization is failing.