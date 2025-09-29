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

### Core Classes

- **DriveshaftToMPHHandler**: Central controller that processes engine and driveshaft RPM to determine gear and speed
- **GearIndicator**: Servo-controlled gear position display with smooth transitions and intelligent gear detection
- **SpeedometerWheel**: Stepper motor speedometer with optical endstop homing and smooth movement
- **DriveshaftInterruptHandler**: Handles driveshaft RPM measurement via optical sensor interrupts
- **EngineRPMInterruptHandler**: Handles engine RPM measurement via optical sensor interrupts
- **DisplayManager**: Manages OLED display output for system status and diagnostics

### Key Features Implemented

- **Intelligent Gear Detection**: GearIndicator now automatically calculates probable gear based on engine RPM and vehicle speed using actual 1970 MGB transmission ratios
- **Gear Stability Filtering**: 750ms confirmation period prevents gear oscillation during shifts
- **Smooth Servo Transitions**: Gear indicator uses easing functions for realistic mechanical movement
- **GPIO Pin Configuration**: All classes accept GPIO pins in constructors for flexible hardware configuration
- **Comprehensive Speed Calculation**: Multiple methods for speed calculation (getCurrentMPH() returns int, getCurrentSpeedMPH() returns float)
- **Vehicle Physics Simulation**: Uses actual tire diameter, differential ratio, and transmission ratios for realistic calculations

### Update Methods

- All classes use update() methods for non-blocking operation in main loop
- Main loop calls both gear calculation and servo update methods for complete functionality

### Recent Updates (Version 2.0.0+)

- Added automatic gear detection based on RPM ratio analysis
- Enhanced GearIndicator with vehicle specifications and transmission logic
- Implemented getCurrentSpeedMPH() method for floating-point speed precision
- Updated constructor patterns to accept GPIO pins for all sensor classes
- Integrated engine RPM monitoring for complete drivetrain simulation