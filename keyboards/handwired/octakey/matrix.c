/*
Copyright 2020 Yax
Copyright 2025 QMK Contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Octakey 2x2 Matrix Implementation using MCP23017 I/O Expander
 *
 * Hardware Configuration:
 * - STM32F411 (Blackpill) on I2C1 (B6=SCL, B7=SDA)
 * - MCP23017 I2C address: 0x20 (A0-A2 tied to ground)
 *
 * Matrix Wiring:
 * - Columns (inputs with pull-ups): MCP23017 GPIOA pins 0-1
 * - Rows (outputs): MCP23017 GPIOB pins 0-1
 *
 * Matrix Layout (COL2ROW):
 *     Col0  Col1
 * Row0 [0]   [1]
 * Row1 [2]   [3]
 */

#include "matrix.h"
#include "mcp23018.h"
#include "i2c_master.h"
#include "wait.h"
#include "debug.h"
#include "print.h"

// MCP23017 I2C address
// Default address with A0, A1, A2 tied to GND: 0x20
#define MCP23017_ADDR 0x20

// Status tracking
static uint8_t mcp23017_status = 0;
static uint8_t mcp23017_reset_loop = 0;

// Initialize the MCP23017
static void init_mcp23017(void) {
    // Initialize I2C
    mcp23018_init(MCP23017_ADDR);
    wait_ms(100);

    // Configure GPIOA (pins 0-1) as inputs for columns
    // Bit = 1 means input, bit = 0 means output
    // We use pins 0 and 1 for columns, rest unused
    mcp23017_status = !mcp23018_set_config(MCP23017_ADDR, mcp23018_PORTA, 0b11111111);
    if (mcp23017_status) {
        dprintf("MCP23017: Failed to configure PORTA\n");
        return;
    }

    // Configure GPIOB (pins 0-1) as outputs for rows
    // Pins 0-1 are outputs (0), rest are inputs (1)
    mcp23017_status = !mcp23018_set_config(MCP23017_ADDR, mcp23018_PORTB, 0b11111100);
    if (mcp23017_status) {
        dprintf("MCP23017: Failed to configure PORTB\n");
        return;
    }

    // Set all rows high (inactive) initially
    mcp23017_status = !mcp23018_set_output(MCP23017_ADDR, mcp23018_PORTB, 0xFF);
    if (mcp23017_status) {
        dprintf("MCP23017: Failed to set PORTB output\n");
        return;
    }

    dprintf("MCP23017: Initialized successfully\n");
}

void matrix_init_custom(void) {
    // Enable debug output if CONSOLE is enabled
    debug_enable = true;
    debug_matrix = true;

    dprintf("Octakey matrix init\n");

    // Initialize MCP23017
    init_mcp23017();
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    // If there was an error communicating with MCP23017, try to reinitialize periodically
    if (mcp23017_status) {
        if (++mcp23017_reset_loop == 0) {
            // Try to reset once every 256 scans (about once per second)
            dprintf("MCP23017: Attempting reconnection...\n");
            init_mcp23017();
            if (mcp23017_status) {
                dprintf("MCP23017: Still not responding\n");
            } else {
                dprintf("MCP23017: Reconnected!\n");
            }
        }
    }

    // If MCP23017 is not responding, return no changes
    if (mcp23017_status) {
        return false;
    }

    // Scan each row
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        // Select the current row by setting it LOW, all others HIGH
        // Row bits are on GPIOB pins 0-1
        uint8_t row_mask = 0xFF & ~(1 << row);

        if (!mcp23018_set_output(MCP23017_ADDR, mcp23018_PORTB, row_mask)) {
            // Failed to set row
            mcp23017_status = 1;
            continue;
        }

        // Small delay for signal to settle
        wait_us(5);

        // Read columns from GPIOA
        uint8_t col_data = 0;
        if (!mcp23018_read_pins(MCP23017_ADDR, mcp23018_PORTA, &col_data)) {
            // Failed to read columns
            mcp23017_status = 1;
            continue;
        }

        // Invert the data (pressed keys read as 0, we want them as 1)
        // Mask to only use the 2 column pins (bits 0-1)
        matrix_row_t current_row_value = (~col_data) & 0b00000011;

        // Check if this row has changed
        if (current_matrix[row] != current_row_value) {
            current_matrix[row] = current_row_value;
            changed = true;
        }
    }

    // Deselect all rows
    mcp23018_set_output(MCP23017_ADDR, mcp23018_PORTB, 0xFF);

    return changed;
}
