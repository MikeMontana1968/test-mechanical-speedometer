# Documentation

This directory contains comprehensive documentation for the ESP32 Mechanical Speedometer project.

## Contents

### [Wiring Diagram](wiring-diagram.md)
Complete wiring instructions and ASCII diagrams showing:
- Pin assignments and connections for all components
- Power distribution and grounding scheme
- Detailed assembly notes and troubleshooting guide
- Visual ASCII diagrams of component layouts

### [Fritzing Parts List](fritzing-parts-list.md)
Instructions for creating professional Fritzing diagrams:
- Required Fritzing parts and substitutions
- Detailed connection tables for breadboard/schematic views
- Custom part creation guidelines
- Export settings for documentation and manufacturing

## Hardware Overview

The project uses these main components connected to an ESP32:

- **Servo Motor** (GPIO 18) - Gear position indicator with 5 positions
- **Stepper Motor** (GPIO 19-23) - Speedometer needle with optical homing
- **Optical Endstop** (GPIO 5) - Home position detection for speedometer
- **OLED Display** (Built-in) - Status and debugging information

## Power Requirements

- **5V Supply**: Servo motor and stepper motor driver
- **3.3V Supply**: ESP32 and optical sensor (from ESP32 regulator)
- **Total Current**: ~300mA peak (stepper + servo active)

## Files in Parent Directory

- `src/config.h` - Pin definitions and hardware constants
- `README.md` - Main project documentation
- `CLAUDE.md` - Development guidelines and specifications

## Creating Physical Diagrams

To create actual Fritzing (.fzz) files:

1. Install [Fritzing](https://fritzing.org/) software
2. Follow the parts list in `fritzing-parts-list.md`
3. Use the connection tables for accurate wiring
4. Export diagrams for documentation or PCB manufacturing

The ASCII diagrams in `wiring-diagram.md` provide a complete text-based reference that's always accessible without specialized software.

## Mechanical Integration

This documentation focuses on electrical connections. For mechanical assembly:

- Mount stepper motor behind speedometer dial
- Position servo for gear indicator movement
- Align optical endstop to detect home position marker
- Ensure adequate clearance for full range of motion

Refer to the main project README for complete build instructions.