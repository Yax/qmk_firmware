# octakey

A 2x2 macropad using an I/O expander for the matrix.

## Hardware

- **MCU**: Blackpill STM32F411
- **RGB LEDs**: SK6812
- **OLED Display**: SSD1306
- **I/O Expander**: MCP23017

## MCP23017 Wiring

### I2C Connection (STM32F411 to MCP23017)
- **SCL**: B6 → MCP23017 pin 12 (SCL)
- **SDA**: B7 → MCP23017 pin 13 (SDA)
- **I2C Address**: 0x20 (A0, A1, A2 tied to GND)

### Matrix Wiring (MCP23017)
- **Columns** (inputs with pull-ups):
  - Col 0: MCP23017 GPA0 (pin 21)
  - Col 1: MCP23017 GPA1 (pin 22)
- **Rows** (outputs):
  - Row 0: MCP23017 GPB0 (pin 1)
  - Row 1: MCP23017 GPB1 (pin 2)

### MCP23017 Power
- VDD (pin 9): 3.3V
- VSS (pin 10): GND
- RESET (pin 18): 3.3V (or 10kΩ pull-up to VDD)

## 2x2 Macropad
2x2 test macro pad.
