# MENU_DRIVEN_RTC_CONFIGURATION_AND_SCHEDULED_DEVICE_CONTROL_SYSTEM
The system continuously monitors the current time, day and date and displays it on a 16x2 LCD display. The device (indicated by an LED) operates automatically based on the ON and OFF times set by the user. If the ON time is later than the OFF time (for example 10:00 PM to 06:00 AM), the system also effectively handles cross-midnight scheduling.
# Menu-Driven RTC Configuration and Scheduled Device Control System

An industry-standard, modular **Embedded-C firmware application** developed for the **LPC2148 / LPC2129 (ARM7TDMI-S)** microcontroller. The system features a robust, menu-driven interface to configure real-time clock (RTC) settings and schedule automated device control profiles based on absolute time intervals.

---

## 📌 Project Overview
The system acts as an automated timer control unit. In normal operation, it continuously pulls parameters from the microcontroller's internal Real-Time Clock (RTC) and displays the running Time (HH:MM:SS), Date (DD/MM/YYYY), and Day-of-Week onto a 16x2 Character LCD. 

When a user triggers a dedicated hardware configuration switch, the system transitions into an interactive menu. Using a 4x4 Matrix Keypad, the user can modify the current time/date or adjust the device's programmed ON/OFF schedule. An external peripheral (represented by an LED) is automatically switched ON and OFF according to this user-defined window.

---

## 🛠️ Hardware Requirements & Environment Layout

### Hardware Components
*   **Microcontroller:** LPC2148 / LPC2129 (ARM7 Core)
*   **Display:** 16x2 Character LCD
*   **Input Interface:** 4x4 Matrix Keypad & Push-button Configuration Switch
*   **Target Output Device:** Active-High LED Status Indicator
*   **Clock Source:** 12MHz External Crystal (FOSC) multiplied via internal PLL to establish Core Clock (CCLK = 60MHz) and Peripheral Clock (PCLK = 15MHz)

### System Pin Mapping (Per `defines.h`)

| Peripheral Module | LPC21xx Pin Mapping | Configured Function / State |
| :--- | :--- | :--- |
| **LCD Data Bus** | `P0.8` to `P0.15` | 8-Bit Data Bus Outputs |
| **LCD Control Pins** | `P0.16` (RS), `P0.18` (RW), `P0.17` (EN) | Driver Control Lines |
| **Keypad Rows** | `P1.16` (R0), `P1.17` (R1), `P1.18` (R2), `P1.19` (R3) | Row Scan Outputs |
| **Keypad Columns** | `P1.20` (C0), `P1.21` (C1), `P1.22` (C2), `P1.23` (C3) | Column Read Inputs (Internal Pull-ups) |
| **Target Device (LED)**| `P0.19` | Active-High Output Profile |
| **Hardware Switch** | `P0.2` | External Interrupt 1 (`EINT1`) Vector Source |

---
##Circuit Diagram
https://github.com/DhruvanaSarmaMaddipati/MENU_DRIVEN_RTC_CONFIGURATION_AND_SCHEDULED_DEVICE_CONTROL_SYSTEM/issues/1#issue-6120213807

## ⚙️ Core Features & Functionality

### 1. Interrupt-Driven Menu Activation
The application avoids busy-waiting polling loops for menu requests. The configuration switch is tied to **External Interrupt 1 (EINT1)**. Pressing the switch instantly invokes an Asynchronous Interrupt Service Routine (ISR) that asserts a volatile `menu_req_flag`, signaling the foreground loop to transition safely into the configuration directory.

### 2. Algorithmic Input Validation
When altering the internal RTC clock settings, the system handles strict sanity boundaries:
*   Hours are limited to `0-23`, Minutes and Seconds to `0-59`.
*   Calendar dates are validated dynamically using a lookup table matching days to their respective months.
*   Includes **Leap-Year Calculation Logic** to automatically expand February's validation array to 29 days when appropriate.

### 3. Absolute Cross-Midnight Scheduling
To prevent unexpected race conditions or state glitches during rollover, time parameters are translated into **absolute elapsed minutes** from 00:00:
\[\text{Total Minutes} = (\text{Hours} \times 60) + \text{Minutes}\]
This allows the firmware to compute standard daytime window logic as well as cross-midnight overnight schedules gracefully:
*   *Daytime Window (e.g., ON 09:00 → OFF 17:00):* Device triggers ON if ON ≤ Current < OFF.
*   *Overnight Window (e.g., ON 22:00 → OFF 06:00):* Device triggers ON if Current ≥ ON **OR** Current < OFF.

---

## 🔄 How the Project Works (System Execution Flow)

The firmware operates on a foreground-background architecture combining an asynchronous Hardware Interrupt Service Routine (ISR) with a synchronous infinite Super-Loop (`while(1)`). The operational logic executes through three distinct phases:

### Phase 1: System Initialization & Boot Setup
Upon power-on or hardware reset, the controller executes setup routines in `main()` to configure the pin directions and initial states:
1. **GPIO Configuration:** Sets up `P0.19` as a push-pull output to drive the LED (Target Device) and sets it to a default low (OFF) state.
2. **Peripheral Bus Setup:** Initializes the 16x2 LCD into 8-bit mode and registers individual columns of the 4x4 Keypad with internal pull-up resistors.
3. **RTC Prescaler Calibration:** Configures the internal real-time clock prescaler registers (`PREINT` and `PREFRAC`) using a 12MHz crystal oscillator to output exactly 32.768 kHz to clock the hardware counter array.
4. **Vectored Interrupt Controller (VIC) Mapping:** Configures `P0.2` as External Interrupt 1 (`EINT1`). It maps the interrupt channel to VIC Slot 0 and registers `eint0_isr` as the targeting handler to catch falling edge transitions.

### Phase 2: Normal Operation Mode (Foreground Loop)
During standard operation, when no user input or interrupt is requested (`menu_req_flag == false`), the controller drives a non-blocking execution cycle:
* **Real-Time Tracking:** The hardware reads the updated contents of the internal RTC registers (`HOUR`, `MIN`, `SEC`, `DOM`, `MONTH`, `YEAR`, `DOW`) sequentially.
* **LCD Refresh Cycle:** Renders the running clock details simultaneously on the display (Line 1 displays Time and Day, Line 2 displays the Calendar Date).
* **Automated Device Control:** Calls `check_cntrl_LED()`, which converts the current time and schedule boundaries into absolute elapsed minutes from midnight. It continuously asserts or clears `P0.19` (LED) depending on whether the current minute counter falls within the programmed operational window.
* **Low-Power Delay:** Introduces a calibrated `delay_ms(200)` step to prevent screen flickering while keeping the response time tight.

### Phase 3: Interactive Configuration Mode (Interrupt Context Switch)
When a user presses the physical setup switch, the asynchronous execution context shifts dynamically:
1. **Interrupt Trigger:** The falling edge on `P0.2` triggers `EINT1`. The processor halts the main thread and jumps straight into `eint0_isr()`.
2. **Flag Assertion:** The ISR updates the volatile variable `menu_req_flag = true`, clears the hardware interrupt source register, and jumps back to the main thread.
3. **Menu Routine Trapping:** In the next loop iteration, the conditional statement blocks normal execution and routes processing exclusively inside `execution_menu_loop()`.
4. **User Sub-Menus Navigation:** 
   * The LCD clears to present option directories (`1:EdRTC`, `2:Sched`). 
   * The controller waits via `key_scan()` for keypad matrix column drops.
   * If modifying parameters, the system redirects to `input_numeric_value()`, processes the input digits array, feeds the data to validation routines (like leap year/month edge testing), and updates the internal RTC or schedule variables safely.
5. **Context Exit:** Selecting the exit option or completing a setup profile resets `menu_req_flag = false`, completely clearing the display to resume normal background scheduling.

---

## 📂 Modular Architecture & Directory Structure

The repository enforces strict separation of concerns, ensuring high code readability and ease of independent software testing:

```text
├── main.c / mini_proj_test.c  # Application entry point, system initialization, foreground super-loop
├── mini_proj.c                # Sub-system driver implementations (LCD, Keypad, RTC, Menu State Machine)
├── mini_proj.h                # Functional prototypes, shared data structures, extern flags declarations
├── defines.h                  # Hardware bit-manipulation macros, Command codes, and Pin mappings
├── types.h                    # Platform-independent standard primitive type definitions (u8, u32, s32)
├── delay.c / delay.h          # Hardware-calibrated microsecond, millisecond, and second busy-delay utilities
└── README.md                  # Comprehensive project documentation
```

### Principal Data Structures
```c
typedef struct {
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
} Timeschedule;
```

---

## 🚀 Execution & Compilation Notes
1. Open the project profile within the **Keil µVision IDE**.
2. Ensure the correct target processor option (`LPC2148` or `LPC2129`) is selected to define appropriate register configurations.
3. Build the source project tree to generate the production executable binary (`.hex`).
4. Connect the target board via a USB-to-UART converter cable.
5. Deploy the compiled binary file to the microcontroller utilizing the **Flash Magic Utility**.
