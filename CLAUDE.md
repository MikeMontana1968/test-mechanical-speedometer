# CLAUDE.md

This file provides guidance to Claude Code when working with this ESP32 mechanical speedometer project.

## Hardware Requirements

- **Microcontroller**: ESP32 Development Board Integrated 0.96 Inch OLED Display, CH340 Driver, WiFi+BLE Wireless Module
- **Optical Endstop**: MakerHawk 6pcs Optical Endstop With 1M Cable Switch Sensor Photoelectric Light Control Limit Switch Module
- **Stepper Motor**: 28BYJ-48 ULN2003 5V Stepper Motor + ULN2003 Driver Board
- **Servo Motor**: DM-S0020 2.1g Ultra-Micro Servo Coreless Motor 2g Digital Servo with JR Connector

## Vehicle Specifications

- **Target Vehicle**: 1970 MGB with three-speed manual transmission (no overdrive)
- **Transmission Ratios**:
  - Reverse: 3.44:1
  - 1st Gear: 3.44:1
  - 2nd Gear: 2.21:1
  - 3rd Gear: 1.37:1
- **Differential Ratio**: 3.9:1
- **Tire Size**: 165-80R13 (23" diameter)

## Code Style Guidelines

- In C++ constructors, put all initializer methods on their own line, indented one tab space
- Use smooth easing transitions for mechanical components (800ms for servo, 1200ms for stepper)
- Implement stability timeouts for gear detection (750ms confirmation period)
- Follow defensive programming practices with input validation and error handling

## Architecture Notes

- **DriveshaftRPMHandler**: Central controller that processes engine and driveshaft RPM to determine gear and speed
- **GearIndicator**: Servo-controlled gear position display with smooth transitions
- **SpeedometerWheel**: Stepper motor speedometer with optical endstop homing and smooth movement
- All classes use update() methods for non-blocking operation in main loop