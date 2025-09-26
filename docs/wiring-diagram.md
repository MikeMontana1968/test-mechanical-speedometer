# ESP32 Mechanical Speedometer - Wiring Diagram

## Overview
This document provides complete wiring instructions for the ESP32 mechanical speedometer project targeting a 1970 MGB restoration.

## Components List

### Main Controller
- **ESP32 Development Board** with integrated 0.96" OLED display

### Motors & Actuators
- **28BYJ-48 Stepper Motor** with ULN2003 driver board (Speedometer)
- **DM-S0020 Ultra-Micro Servo** (Gear Indicator)

### Sensors
- **Optical Endstop** with 1M cable (Speedometer Home Position)
- **Optical Endstop** with 1M cable (Driveshaft RPM Sensor)

### Power Supply
- **5V Power Supply** (for stepper motor and servo)
- **ESP32 USB Power** or external 3.3V supply

## Pin Assignments

| Component | ESP32 Pin | Function | Notes |
|-----------|-----------|----------|--------|
| **OLED Display** | | | |
| SDA | GPIO 4 | I2C Data | Built-in on ESP32 dev board |
| SCL | GPIO 15 | I2C Clock | Built-in on ESP32 dev board |
| RST | GPIO 16 | Reset | Built-in on ESP32 dev board |
| **Servo Motor** | | | |
| Signal | GPIO 19 | PWM Control | Orange/Yellow wire |
| VCC | 5V | Power | Red wire |
| GND | GND | Ground | Brown/Black wire |
| **Stepper Motor** | | | |
| IN1 | GPIO 25 | Phase 1 | ULN2003 driver input |
| IN2 | GPIO 26 | Phase 2 | ULN2003 driver input |
| IN3 | GPIO 27 | Phase 3 | ULN2003 driver input |
| IN4 | GPIO 32 | Phase 4 | ULN2003 driver input |
| **Optical Endstop** | | | |
| Signal | GPIO 5 | Digital Input | Speedometer home position |
| VCC | 3.3V | Power | ESP32 3.3V output |
| GND | GND | Ground | ESP32 ground |
| **Driveshaft Sensor** | | | |
| Signal | GPIO 18 | Digital Input | Driveshaft RPM measurement |
| VCC | 3.3V | Power | ESP32 3.3V output |
| GND | GND | Ground | ESP32 ground |

## Wiring Diagram (ASCII)

```
                    ESP32 Development Board
                   ┌─────────────────────────┐
                   │    ┌─────────────┐      │
                   │    │ 0.96" OLED  │      │
                   │    │   Display   │      │
                   │    └─────────────┘      │
   ┌───────────────┤ GPIO 4  (SDA)          │
   │          ┌────┤ GPIO 15 (SCL)          │
   │          │ ┌──┤ GPIO 16 (RST)          │
   │          │ │  │                        │
   │          │ │  │ GPIO 18 ├──────────────┼─── Driveshaft Sensor Signal
   │          │ │  │ GPIO 19 ├──────────────┼─── Servo Signal (Orange)
   │          │ │  │ GPIO 25 ├──────────────┼─── Stepper IN1
   │          │ │  │ GPIO 26 ├──────────────┼─── Stepper IN2
   │          │ │  │ GPIO 27 ├──────────────┼─── Stepper IN3
   │          │ │  │ GPIO 32 ├──────────────┼─── Stepper IN4
   │          │ │  │ GPIO 5  ├──────────────┼─── Endstop Signal
   │          │ │  │         │              │
   │          │ │  │ 5V      ├──────────────┼─── 5V Power Rail
   │          │ │  │ 3.3V    ├──────────────┼─── 3.3V Power Rail
   │          │ │  │ GND     ├──────────────┼─── Ground Rail
   │          │ │  └─────────────────────────┘
   │          │ │
   │          │ └─ Built-in OLED connections
   │          └─ Built-in OLED connections
   └─ Built-in OLED connections

    DM-S0020 Servo               28BYJ-48 + ULN2003 Driver
   ┌──────────────┐             ┌─────────────────────────────┐
   │   ┌──────┐   │             │  ULN2003 Driver Board      │
   │   │      │   │             │ ┌─────┐  ┌─────────────┐   │
   │   │ Gear │   │             │ │IN1  │  │             │   │◄─── GPIO 25
   │   │ Dial │   │             │ │IN2  │  │   28BYJ-48  │   │◄─── GPIO 26
   │   │      │   │             │ │IN3  │  │   Stepper   │   │◄─── GPIO 27
   │   └──────┘   │             │ │IN4  │  │             │   │◄─── GPIO 32
   │              │             │ │5V   │  │             │   │◄─── 5V
   │ ┌──────────┐ │             │ │GND  │  └─────────────┘   │◄─── GND
   │ │ ●   ●  ● │ │             │ └─────┘                   │
   │ └──────────┘ │             └─────────────────────────────┘
   │  1   2  3    │               3-Pin RC Servo Connector
   │  │   │  │    │               Pin 1: Brown/Black (GND)
   │  │   │  └────┼─── GPIO 19    Pin 2: Red (5V Power)
   │  │   └───────┼─── 5V         Pin 3: Orange/Yellow (Signal)
   │  └───────────┼─── GND
   └──────────────┘
                                              │
                        ┌─────────────────────┘
                        │ Speedometer Needle Assembly
                        │ ┌─────────────────────────────┐
                        │ │    ┌─────────┐             │
                        │ │    │ Needle  │             │
                        │ │    │ 0-90MPH │             │
                        │ │    └─────────┘             │
                        │ │                            │
                        │ │  ┌─────────────────────┐   │
                        │ │  │ Optical Endstop    │   │
                        │ │  │                    │   │
                        │ │  │ Signal ─────────── │───┼─── GPIO 5
                        │ │  │ VCC ────────────── │───┼─── 3.3V
                        │ │  │ GND ────────────── │───┼─── GND
                        │ │  └─────────────────────┘   │
                        │ └─────────────────────────────┘
                        └─ Mechanical coupling to stepper

    Power Distribution
   ┌─────────────────┐
   │ 5V Power Supply │
   │                 │
   │ +5V ────────────┼─── Servo VCC, Stepper VCC
   │ GND ────────────┼─── Common Ground
   └─────────────────┘
            │
        ESP32 USB or External 3.3V Supply
```

## Detailed Wiring Instructions

### 1. Power Connections
```
5V Power Supply:
├── Positive (+5V) → ESP32 5V pin
├── Positive (+5V) → Servo Red wire
├── Positive (+5V) → ULN2003 5V pin
└── Negative (GND) → Common ground rail

ESP32 3.3V Output:
└── Optical Endstop VCC pin
```

### 2. Servo Motor (Gear Indicator)

#### RC Servo Connector Pinout (Standard 3-Pin JR/Futaba)
```
DM-S0020 Servo Connector (viewed from wire side):
┌─────────────────┐
│ ●     ●     ●   │  ← 3-pin connector
└─────────────────┘
  1     2     3

Pin 1 (Brown/Black): Ground (GND)
Pin 2 (Red):         Power (VCC) - 4.8V to 6V, typically 5V
Pin 3 (Orange/Yellow/White): Signal (PWM) - 1-2ms pulse width, 50Hz

Wire Color Standards:
├── Standard Colors:    Brown  | Red | Orange
├── Alternative Colors: Black  | Red | Yellow
└── Some Servos:        Black  | Red | White
```

#### ESP32 Connections
```
DM-S0020 Servo → ESP32:
├── Pin 1 (Brown/Black) → Ground rail
├── Pin 2 (Red)         → 5V power rail
└── Pin 3 (Orange)      → GPIO 19 (PWM signal)

Signal Specifications:
├── PWM Frequency: 50Hz (20ms period)
├── Pulse Width Range: 1.0ms to 2.0ms
├── Center Position: 1.5ms pulse width
└── Voltage Level: 3.3V logic (ESP32 output)

Position Range: 0° to 60° (5 positions)
├── 0°  → Reverse    (1.0ms pulse)
├── 15° → Neutral    (1.125ms pulse)
├── 30° → 1st Gear   (1.25ms pulse)
├── 45° → 2nd Gear   (1.375ms pulse)
└── 60° → 3rd Gear   (1.5ms pulse)
```

### 3. Stepper Motor (Speedometer)
```
ULN2003 Driver → ESP32:
├── IN1 → GPIO 25
├── IN2 → GPIO 26
├── IN3 → GPIO 27
├── IN4 → GPIO 32
├── VCC → 5V power rail
└── GND → Ground rail

28BYJ-48 Motor → ULN2003:
└── 5-pin connector to driver board

Speed Range: 0-90 MPH
└── Full revolution = 90 MPH range
```

### 4. Optical Endstop (Home Position Sensor)
```
Optical Endstop → ESP32:
├── Signal → GPIO 5 (with internal pull-up)
├── VCC    → 3.3V output pin
└── GND    → Ground rail

Function: Detects speedometer "home" position (0 MPH)
└── LOW signal when beam is blocked by speed dial marker

Driveshaft Sensor → ESP32:
├── Signal → GPIO 18 (with internal pull-up)
├── VCC    → 3.3V output pin
└── GND    → Ground rail

Function: Measures driveshaft RPM for transmission calculations
└── LOW signal when beam is blocked by driveshaft rotation marker
```

### 5. OLED Display (Built-in)
```
Integrated 0.96" OLED → ESP32 (Pre-wired):
├── SDA → GPIO 4  (I2C Data)
├── SCL → GPIO 15 (I2C Clock)
└── RST → GPIO 16 (Reset)

No external wiring required for display
```

## Assembly Notes

### Mechanical Mounting
1. **Speedometer Assembly**: Mount stepper motor behind speedometer dial
2. **Gear Indicator**: Mount servo with mechanical linkage to gear position indicator
3. **Endstop Positioning**: Align optical sensor to detect home marker on speed dial
4. **Cable Management**: Use 1M cables for sensors to allow flexible mounting

### Electrical Considerations
1. **Power Supply**: Ensure 5V supply can handle stepper motor current (~150mA) plus servo (~100mA)
2. **Grounding**: Use star grounding topology with ESP32 as central ground point
3. **Signal Integrity**: Keep PWM and digital signal wires away from power lines
4. **Pull-up Resistors**: Optical endstop uses ESP32 internal pull-up (no external resistor needed)

### Testing Procedure
1. **Power Check**: Verify 5V and 3.3V levels before connecting components
2. **Servo Test**: Verify smooth movement through all 5 gear positions
3. **Stepper Test**: Confirm full rotation and smooth operation
4. **Endstop Test**: Verify digital signal changes when beam is blocked
5. **System Integration**: Test complete speedometer calibration sequence

## Troubleshooting

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| Servo jitters | Insufficient power | Check 5V supply current capacity |
| Stepper skips steps | Wrong wiring sequence | Verify IN1-IN4 pin connections |
| Endstop always reads HIGH | No pull-up enabled | Check GPIO 5 internal pull-up in code |
| Display not working | I2C connection issue | Verify built-in OLED is functional |

## File References
- Main configuration: `src/config.h`
- Pin definitions: Lines 4-32 in config.h
- Hardware initialization: Each class `begin()` method

This wiring diagram supports the complete mechanical speedometer system with authentic 1970 MGB specifications and smooth operation.