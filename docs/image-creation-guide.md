# Image Creation Guide

This guide explains how to create the wiring diagram images referenced in the main README.md.

## Required Images

### 1. `wiring-diagram.png`
**System Overview Diagram**
- **Size**: 1200x800 pixels recommended
- **Format**: PNG with transparent background
- **Content**: Complete system showing ESP32, stepper motor, servo, and optical endstop
- **Style**: Clean schematic with labeled connections

### 2. `fritzing-breadboard.png`
**Breadboard Layout**
- **Size**: 1000x600 pixels recommended
- **Format**: PNG with white background
- **Content**: Fritzing breadboard view showing physical component placement
- **Style**: Realistic component representation

## Creating the Images

### Option 1: Fritzing Software
1. **Install Fritzing**: Download from https://fritzing.org/
2. **Create Circuit**: Use parts from `fritzing-parts-list.md`
3. **Export Images**:
   - Breadboard View → Export → PNG → `fritzing-breadboard.png`
   - Schematic View → Export → PNG → `wiring-diagram.png`

### Option 2: Draw.io (Free Online)
1. **Access**: Go to https://app.diagrams.net/
2. **Template**: Use "Electrical" template
3. **Components**: Add ESP32, motors, sensors with proper symbols
4. **Connections**: Draw wires with labels matching pin assignments
5. **Export**: File → Export as → PNG

### Option 3: KiCad (Professional)
1. **Schematic**: Create professional schematic with proper symbols
2. **Layout**: Optional PCB layout for advanced documentation
3. **Export**: Plot → PNG with high resolution

### Option 4: Hand-drawn + Scan
1. **Draw**: Create neat hand-drawn diagrams on grid paper
2. **Scan**: High resolution scan (300 DPI minimum)
3. **Edit**: Clean up in image editor if needed

## Image Specifications

### File Naming
```
docs/wiring-diagram.png     - System schematic
docs/fritzing-breadboard.png - Breadboard layout
```

### Content Requirements

#### Wiring Diagram (`wiring-diagram.png`)
- ESP32 with all pin labels clearly visible
- Stepper motor + ULN2003 driver with 4-wire connection
- Servo motor with 3-wire connection (signal, power, ground)
- Optical endstop with 3-wire connection
- Power supply connections (5V and 3.3V rails)
- Wire colors matching the connection tables
- Component labels and pin numbers

#### Fritzing Breadboard (`fritzing-breadboard.png`)
- Realistic breadboard with power rails
- ESP32 development board properly positioned
- Physical component placement matching actual assembly
- Jumper wires showing actual routing
- Color-coded connections for easy following

## Image Optimization

### For README Display
- **Resolution**: 96-150 DPI for web viewing
- **Compression**: Moderate PNG compression for smaller file size
- **Dimensions**: Max 1200px wide (scales automatically on GitHub)

### File Size Guidelines
- Target: < 500KB per image for fast loading
- Maximum: < 1MB to avoid GitHub display issues

## Alternative Text Diagrams

If creating images isn't feasible, the ASCII diagrams in `wiring-diagram.md` provide complete technical information. The README can reference these text diagrams:

```markdown
## Wiring Reference
See [detailed ASCII wiring diagrams](docs/wiring-diagram.md) for complete connection information.
```

## Adding Images to Repository

Once images are created:

1. **Save to docs folder**: `docs/wiring-diagram.png`, `docs/fritzing-breadboard.png`
2. **Commit to git**: Images will be available on GitHub
3. **Verify display**: Check README.md on GitHub shows images correctly

## Markdown Image Syntax

Current README.md uses:
```markdown
![Wiring Diagram](docs/wiring-diagram.png)
![Fritzing Diagram](docs/fritzing-breadboard.png)
```

This will automatically display the images when the files exist in the docs folder.

## Professional Tips

- **Consistency**: Use same color scheme and style for both images
- **Labels**: Make all text large enough to read when scaled down
- **Clarity**: Prefer simple, clean lines over complex artistic elements
- **Standards**: Follow electrical schematic conventions for symbols
- **Accessibility**: Include descriptive alt text in markdown

The images will greatly enhance the project documentation and make assembly much easier for users.