# Advanced-Motorized-Curtain-Control-with-UART-Limit-Switch-Debouncing-AVR-
This AVR-based embedded system automates the opening and closing of a curtain using UART serial commands and physical **limit switches**. The system implements **debounce logic** to avoid false triggers from mechanical bounce and maintains **state awareness** to prevent redundant motor operations.

---

## Project Overview

- **Microcontroller**: AVR (e.g., ATmega2560)
- **Clock Speed**: 8 MHz
- **Control Interface**: UART serial terminal (e.g., PuTTY, Arduino Serial Monitor)
- **Actuation**: H-Bridge motor control for opening/closing curtains
- **Sensors**: Digital limit switches (fully open, fully closed detection)
- **Key Features**:
  - UART-based command parsing (`open curtain`, `close curtain`, `stop curtain`)
  - Debounced limit switch detection
  - Curtain position tracking (fully open, fully closed)
  - Prevents redundant commands (e.g., avoids opening an already open curtain)

---

## UART Command Set

| Command           | Description                             |
|------------------|-----------------------------------------|
| `open curtain`   | Opens curtain (if not already open)      |
| `close curtain`  | Closes curtain (if not already closed)   |
| `stop curtain`   | Stops the motor at its current position  |
| _Other commands_ | Returns `"Unknown command!"`             |

All commands are case-insensitive and terminated by `\r` or `\n`.

---

## System Behavior

- When you type `open curtain`, the motor starts rotating in the **open direction** until the **open limit switch** is triggered. At that point:
  - Motor stops
  - `"Curtain fully open.\r\n"` is printed to UART
- If `open curtain` is received while already open, it prints `"Curtain is already open.\r\n"` and does nothing
- Same logic applies for closing with `close curtain`
- You can manually halt movement mid-way with `stop curtain`

---

## Debounce Logic

To eliminate mechanical bouncing noise on the limit switches:
- The system waits for **20 consecutive milliseconds** of a valid switch press before confirming
- Uses counters (`open_switch_counter`, `close_switch_counter`) and booleans (`*_triggered`) to ensure reliability

---

## Circuit Setup in proteus
<img width="817" height="658" alt="image" src="https://github.com/user-attachments/assets/e2b1a9e9-fc08-44e3-9c0b-95ce59b7d19b" />
