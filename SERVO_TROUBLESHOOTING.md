# Servo Troubleshooting Guide - GPIO 18 PWM Output Issue

## Issue Description
No digital output observed on GPIO 18 when using an oscilloscope to measure the servo control signal.

## Root Cause Analysis

The original code had several issues that could prevent PWM output:

1. **No Immediate Output**: Servo only received commands during gear transitions
2. **Limited Debug Info**: Insufficient logging to verify servo initialization
3. **No Continuous Output**: PWM stopped between transitions

## Fixes Applied

### 1. Enhanced Servo Initialization (`GearIndicator.cpp`)

**Added:**
- Detailed debug output during servo initialization
- Servo attachment verification with error checking
- Immediate servo positioning after initialization
- 100ms delay after initial positioning

```cpp
void GearIndicator::begin() {
    Serial.println("Initializing gear indicator servo...");
    Serial.print("Servo pin: GPIO ");
    Serial.println(SERVO_PIN);

    // Configure servo with custom pulse width range
    gearServo.setPeriodHertz(50);  // Standard 50Hz servo frequency

    if (gearServo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
        Serial.println("Servo attached successfully");
    } else {
        Serial.println("ERROR: Servo attach failed!");
        return;
    }

    // Initialize to neutral position immediately
    currentAngle = GEAR_ANGLES[NEUTRAL];
    Serial.print("Setting servo to neutral angle: ");
    Serial.print(currentAngle);
    Serial.println(" degrees");

    gearServo.write(currentAngle);
    delay(100);  // Give servo time to move

    isInitialized = true;
}
```

### 2. Added Servo Test Function (`GearIndicator.cpp`)

**New Function:** `testServoOutput()` - Provides immediate servo testing for scope verification:

```cpp
void GearIndicator::testServoOutput() {
    Serial.println("=== SERVO OUTPUT TEST FOR SCOPE VERIFICATION ===");

    // Test each gear position with 3-second holds
    int testAngles[] = {0, 15, 30, 45, 60};
    const char* testNames[] = {"Reverse", "Neutral", "1st Gear", "2nd Gear", "3rd Gear"};

    for (int i = 0; i < 5; i++) {
        Serial.print("Setting servo to ");
        Serial.print(testAngles[i]);
        Serial.print(" degrees (");
        Serial.print(testNames[i]);
        Serial.println(")");

        gearServo.write(testAngles[i]);
        Serial.println(">>> Check scope now! PWM should be active on GPIO 18 <<<");
        delay(3000);  // 3 seconds per position for scope measurement
    }

    // Test extreme positions
    Serial.println("Testing minimum angle (0 degrees):");
    gearServo.write(0);
    delay(2000);

    Serial.println("Testing maximum angle (180 degrees):");
    gearServo.write(180);
    delay(2000);

    // Return to neutral
    Serial.println("Returning to neutral (15 degrees):");
    gearServo.write(15);
}
```

### 3. Enhanced Debug Output

**Added continuous debug during transitions:**
```cpp
void GearIndicator::updateServoPosition() {
    if (!isInitialized) {
        Serial.println("ERROR: Servo not initialized");
        return;
    }

    gearServo.write(currentAngle);

    // Debug output every 100ms during transitions
    static unsigned long lastDebugTime = 0;
    unsigned long now = millis();
    if (now - lastDebugTime > 100 && isMoving) {
        Serial.print("Servo angle: ");
        Serial.print(currentAngle);
        Serial.println(" degrees");
        lastDebugTime = now;
    }
}
```

### 4. Automatic Servo Test in Setup (`main.cpp`)

**Added immediate test execution:**
```cpp
// Test servo output immediately after initialization
Serial.println("Testing servo output on GPIO 18...");
gearIndicator.testServoOutput();
Serial.println("Servo test complete. Check scope for PWM signal.");
```

## Expected PWM Signal Characteristics

### Signal Specifications
- **Frequency**: 50Hz (20ms period)
- **Voltage**: 3.3V logic level (ESP32 output)
- **Pulse Width Range**:
  - 0° → ~1.0ms pulse width
  - 15° → ~1.125ms pulse width
  - 90° → ~1.5ms pulse width (center)
  - 180° → ~2.0ms pulse width

### Scope Settings for Measurement
- **Timebase**: 5ms/div (to see full 20ms period)
- **Voltage**: 1V/div (for 3.3V logic)
- **Trigger**: Rising edge, ~1.65V threshold
- **Coupling**: DC coupling recommended

## Troubleshooting Steps

### Step 1: Check Serial Debug Output
Upload the fixed firmware and monitor serial output at 115200 baud. You should see:
```
Initializing gear indicator servo...
Servo pin: GPIO 18
Servo attached successfully
Setting servo to neutral angle: 15 degrees
Gear indicator initialized successfully
=== SERVO OUTPUT TEST FOR SCOPE VERIFICATION ===
```

### Step 2: Verify Hardware Connections
- **GPIO 18**: Connect scope probe here
- **Ground**: Connect scope ground to ESP32 GND
- **Power**: Ensure ESP32 is properly powered (USB or external)

### Step 3: Scope Verification
During the test sequence, you should observe:
1. Continuous 50Hz PWM signal
2. Pulse width changes as angles change
3. 3.3V signal amplitude
4. 20ms period consistency

### Step 4: Hardware Isolation Test
If still no output, try this simple test:
```cpp
void setup() {
    Serial.begin(115200);
    pinMode(18, OUTPUT);
}

void loop() {
    digitalWrite(18, HIGH);
    delay(1);
    digitalWrite(18, LOW);
    delay(19);
}
```
This should produce a simple ~50Hz square wave on GPIO 18.

## Potential Hardware Issues

### ESP32 Pin Damage
- GPIO 18 may be damaged from overvoltage/overcurrent
- Test with different GPIO pin (modify `#define SERVO_PIN` in config.h)

### Scope Probe Issues
- Check probe compensation
- Verify probe ground connection
- Try 1x probe setting instead of 10x

### Power Supply Issues
- Insufficient power can cause GPIO malfunction
- Check 3.3V rail voltage under load
- Try external 3.3V supply for ESP32

### Library Issues
- ESP32Servo library version compatibility
- Try different servo library (built-in Servo library)

## Alternative Testing Methods

### LED Test
Replace servo connection with LED + resistor:
```cpp
// In begin() method, add:
pinMode(SERVO_PIN, OUTPUT);
digitalWrite(SERVO_PIN, HIGH);  // LED should light
delay(1000);
digitalWrite(SERVO_PIN, LOW);   // LED should turn off
```

### Multimeter Test
Set multimeter to measure frequency on GPIO 18 during servo operation.
Should read ~50Hz.

## Next Steps if Issue Persists

1. **Try Different GPIO Pin**: Change `SERVO_PIN` to GPIO 16 or 17
2. **Use Built-in Servo Library**: Replace ESP32Servo with standard Arduino Servo
3. **Hardware Bypass Test**: Connect servo directly to 3.3V/GND to verify servo operation
4. **ESP32 Replacement**: If other GPIO pins also fail, ESP32 may be damaged

## Files Modified
- `src/classes/GearIndicator.h` - Added testServoOutput() declaration
- `src/classes/GearIndicator.cpp` - Enhanced initialization and added test function
- `src/main.cpp` - Added automatic servo test in setup
- `SERVO_TROUBLESHOOTING.md` - This troubleshooting guide

## Build and Test
```bash
pio run --target upload
pio device monitor --baud 115200
```

The servo test will run automatically on startup, providing immediate feedback for scope measurement.