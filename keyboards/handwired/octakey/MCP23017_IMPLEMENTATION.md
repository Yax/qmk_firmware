# MCP23017 Implementation for Octakey

## Overview

This implementation uses the **official QMK MCP23018 driver** (compatible with MCP23017) to handle the 2x2 matrix through an I2C I/O expander.

## Changes Made

### 1. Created `matrix.c`
Custom matrix implementation using:
- **Driver**: `/drivers/gpio/mcp23018.{c,h}` (official QMK core driver)
- **API**: Modern unified I2C API (`i2c_master.h`)
- **Mode**: `CUSTOM_MATRIX = lite` (less boilerplate)

### 2. Updated `rules.mk`
Added:
```make
CUSTOM_MATRIX = lite
SRC += matrix.c
I2C_DRIVER_REQUIRED = yes
```

### 3. Updated `config.h`
Added I2C pin configuration for STM32F411:
```c
#define I2C1_SCL_PIN B6
#define I2C1_SDA_PIN B7
#define I2C_DRIVER I2CD1
```

### 4. Updated `info.json`
Removed `matrix_pins` (now handled by custom matrix code)

### 5. Updated `readme.md`
Added complete MCP23017 wiring documentation

## Hardware Configuration

### MCP23017 I2C Address
- **Address**: 0x20 (A0, A1, A2 pins tied to GND)
- Can be changed in `matrix.c` if you wire A0-A2 differently

### Pin Assignments

#### STM32F411 → MCP23017
- B6 (I2C1_SCL) → MCP23017 pin 12 (SCL)
- B7 (I2C1_SDA) → MCP23017 pin 13 (SDA)

#### MCP23017 Matrix
- **GPIOA (Port A)**: Columns (inputs with internal pull-ups)
  - GPA0 → Column 0
  - GPA1 → Column 1

- **GPIOB (Port B)**: Rows (outputs, active low)
  - GPB0 → Row 0
  - GPB1 → Row 1

## Technical Details

### Matrix Scanning Method
- **Type**: COL2ROW
- **Scanning**: Rows are driven LOW one at a time, columns are read
- **Debouncing**: Handled by QMK's standard debounce (5ms)
- **Error Recovery**: Auto-reconnect if I2C communication fails

### I2C Configuration
- **Speed**: 400kHz (Fast Mode)
- **Peripheral**: STM32 I2C1
- **Timeout**: 100ms per transaction
- **Pull-ups**: Internal pull-ups on MCP23017 GPIOA

## Advantages Over Previous Implementation

1. **Official Driver**: Uses QMK's maintained `mcp23018` driver
2. **Modern I2C API**: Unified across all platforms
3. **Clean Code**: `CUSTOM_MATRIX = lite` reduces boilerplate
4. **Better Error Handling**: Auto-reconnect on I2C errors
5. **Future-Proof**: Compatible with latest QMK (Nov 2025)
6. **Well-Tested**: Same approach used by 32+ keyboards in QMK

## Comparison to Other Keyboards

### Similar Implementations in QMK:
- **Ferris 0.2**: Split keyboard with MCP23017 on one side
- **ErgoDox EZ**: Uses MCP23018 (same chip family)
- **Moonlander**: ZSA keyboard with MCP23018
- **Sugarglider**: MechWild keyboard with MCP23018

All use the same driver and approach.

## Building

```bash
qmk compile -kb handwired/octakey -km default
```

Or:
```bash
make handwired/octakey:default
```

## Testing

With `CONSOLE_ENABLE = yes` in `rules.mk`, you can see debug messages:
- MCP23017 initialization status
- I2C communication errors
- Reconnection attempts

Use `hid_listen` or QMK Toolbox to view console output.

## Troubleshooting

### MCP23017 Not Responding
1. Check I2C wiring (SCL, SDA, GND, VDD)
2. Verify I2C address (default 0x20)
3. Check pull-up resistors on SCL/SDA (4.7kΩ recommended)
4. Verify 3.3V power to MCP23017
5. Check console output for error messages

### Keys Not Registering
1. Verify matrix wiring to MCP23017 pins
2. Check diode orientation (COL2ROW)
3. Test with multimeter in continuity mode
4. Enable debug output and monitor matrix state

### Wrong Key Mappings
1. Check column/row pin assignments in `matrix.c`
2. Verify physical wiring matches code
3. Use QMK's matrix debug (`#define DEBUG_MATRIX`)

## Future Improvements

Possible enhancements:
1. Use more MCP23017 pins for expanded matrix (up to 8x8)
2. Add interrupt-driven scanning (INTA/INTB pins)
3. Implement pin change detection for lower power consumption
4. Support multiple MCP23017 chips for larger matrices

## References

- [QMK Custom Matrix Documentation](https://docs.qmk.fm/custom_matrix)
- [QMK I2C Driver Documentation](https://docs.qmk.fm/drivers/i2c)
- [MCP23017 Datasheet](http://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf)
- [QMK MCP23018 Driver Source](https://github.com/qmk/qmk_firmware/tree/master/drivers/gpio)
