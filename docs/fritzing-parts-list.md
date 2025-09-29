# Fritzing Parts List and Instructions

## Creating the Fritzing Diagram

To create the actual Fritzing diagram for this project, use these components and connections:

### Required Fritzing Parts

#### Core Components
- **ESP32 Development Board** (Generic ESP32 DevKit)
- **28BYJ-48 Stepper Motor** with ULN2003 driver
- **Micro Servo** (9g servo - similar to DM-S0020)
- **Optical Endstop/Photointerrupter** sensor
- **Breadboard** (half-size recommended)

#### Electronic Components
- **5V Power Supply** (external power jack)
- **Jumper wires** (M-M, M-F types)
- **Resistors**: None required (internal pull-ups used)

### Fritzing Connections Table

| From Component | From Pin | To Component | To Pin | Wire Color |
|----------------|----------|--------------|--------|------------|
| **Power Supply** | | | | |
| 5V Power | +5V | ESP32 | VIN | Red |
| 5V Power | +5V | Servo | VCC | Red |
| 5V Power | +5V | ULN2003 | VCC | Red |
| 5V Power | GND | ESP32 | GND | Black |
| **Servo Motor** | | | | |
| Servo | Signal | ESP32 | GPIO 19 | Orange |
| Servo | VCC | Power Rail | +5V | Red |
| Servo | GND | Power Rail | GND | Brown |
| **Stepper Motor** | | | | |
| ULN2003 | IN1 | ESP32 | GPIO 25 | Blue |
| ULN2003 | IN2 | ESP32 | GPIO 26 | Green |
| ULN2003 | IN3 | ESP32 | GPIO 27 | Yellow |
| ULN2003 | IN4 | ESP32 | GPIO 32 | Purple |
| ULN2003 | VCC | Power Rail | +5V | Red |
| ULN2003 | GND | Power Rail | GND | Black |
| **Optical Endstop** | | | | |
| Endstop | Signal | ESP32 | GPIO 5 | White |
| Endstop | VCC | ESP32 | 3.3V | Red |
| Endstop | GND | ESP32 | GND | Black |
| **Driveshaft Sensor** | | | | |
| Sensor | Signal | ESP32 | GPIO 18 | Gray |
| Sensor | VCC | ESP32 | 3.3V | Red |
| Sensor | GND | ESP32 | GND | Black |
| **Engine RPM Sensor** | | | | |
| Sensor | Signal | ESP32 | GPIO 4 | Blue |
| Sensor | VCC | ESP32 | 3.3V | Red |
| Sensor | GND | ESP32 | GND | Black |

### Fritzing Diagram Layout

#### Breadboard View
```
1. Place ESP32 DevKit spanning the center divide
2. Position ULN2003 driver on the right side
3. Place servo connector on the left side
4. Mount optical sensor on lower breadboard area
5. Use power rails for 5V distribution
6. Connect all grounds to common rail
```

#### Schematic View
```
Power Distribution:
- 5V rail feeding servo and stepper driver
- 3.3V from ESP32 powering optical sensor
- Common ground for all components

Signal Routing:
- GPIO 19: PWM to servo (1000-2000μs pulse width)
- GPIO 25,26,27,32: Digital outputs to stepper driver
- GPIO 5: Digital input from optical endstop (pulled up)
- GPIO 18: Digital input from driveshaft sensor (pulled up)
- GPIO 4: Digital input from engine RPM sensor (pulled up)
```

#### PCB View
```
For custom PCB design:
- ESP32 module in center
- Power regulation circuit (5V to 3.3V)
- Stepper driver connections with pull-down resistors
- Servo connector with signal conditioning
- Sensor inputs with ESD protection
```

### Part Substitutions in Fritzing

Since exact parts may not be available:

#### ESP32 Board
- Use: "ESP32 DevKit" or "NodeMCU ESP32"
- Configure pins to match GPIO assignments

#### Servo Motor
- Use: "Micro Servo 9g" or generic servo
- Ensure signal wire connects to GPIO 18

#### Stepper Motor
- Use: "28BYJ-48" stepper motor part
- Pair with "ULN2003" driver module

#### Optical Sensor
- Use: "Photointerrupter" or "Optical Switch"
- Alternative: Generic 3-pin sensor module

### Custom Part Creation

If specific parts aren't available in Fritzing:

#### DM-S0020 Servo
```xml
<!-- Create custom servo part with these specifications -->
<connector id="signal" type="male" name="Signal Pin">
<connector id="vcc" type="male" name="VCC (+5V)">
<connector id="gnd" type="male" name="Ground">
```

#### ESP32 with OLED
```xml
<!-- Custom part showing integrated OLED display -->
<property name="Display">0.96" OLED Integrated</property>
<property name="I2C">SDA:GPIO21, SCL:GPIO22</property>
```

### Assembly Instructions for Fritzing

1. **Start with Power**: Draw 5V power supply connections first
2. **Add ESP32**: Place centrally with clear pin access
3. **Connect Motors**: Route motor control signals with appropriate wire colors
4. **Add Sensors**: Connect optical endstop with pull-up indication
5. **Label Everything**: Use Fritzing text tool to label all connections
6. **Add Notes**: Include voltage and current requirements

### Export Settings

When exporting from Fritzing:

#### For Documentation
- **PDF**: High resolution for printing
- **PNG**: 300 DPI for web use
- **SVG**: Vector format for scaling

#### For Manufacturing
- **Gerber Files**: If creating custom PCB
- **Parts List**: Export BOM with part numbers
- **Assembly Drawing**: Include component placement

### Additional Documentation

Create these supplementary files in Fritzing:

1. **Power Analysis**: Show current draw for each component
2. **Signal Timing**: Indicate PWM frequencies and timing
3. **Mechanical Notes**: Reference servo mounting and stepper coupling
4. **Testing Points**: Mark critical measurement points

This comprehensive parts list will help recreate the exact wiring diagram in Fritzing for professional documentation and PCB design.