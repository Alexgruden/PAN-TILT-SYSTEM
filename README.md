# Pan-Tilt System

A two-node motion-mirroring system. A wireless handheld node senses its own orientation and sends it over Bluetooth to a controller node, which drives a pan-tilt servo mount so it mirrors the handheld's movement.

Built as a mix of personal interest and my Microcontrollers 2 course.

```
 HANDHELD NODE                                     CONTROLLER NODE
 ESP32 + MPU-6500  ── Bluetooth Classic (SPP) ──▶  HC-05 ── UART ──▶ STM32F411 ── PWM ──▶ pan + tilt servos
```

## Hardware

| Node | Part | Connection |
|---|---|---|
| Handheld | ESP32-WROOM-32 dev board on a breadboard, powered by a USB battery pack | |
| Handheld | MPU-6500 inertial measurement unit (accelerometer + gyroscope) | I2C at 400 kHz, GPIO 21 (data) and GPIO 22 (clock) |
| Controller | STM32F411CEU6 board, flashed and debugged with an ST-LINK V2 | |
| Controller | HC-05 Bluetooth Classic serial module | USART1 at 9600 baud: PA10 (receive), PA9 (transmit) |
| Controller | Prebuilt pan-tilt bracket: 270° pan servo, 180° tilt servo | Timer 3 PWM: PB4 (pan, channel 1), PB5 (tilt, channel 2) |
| Controller | 6 V supply for the servos only | Ground shared with the STM32 |

I2C was chosen for the IMU because it's simple to wire and leaves room to add an OLED display on the same bus.

## Data link

The handheld connects to the HC-05 using Bluetooth Classic with the Serial Port Profile. It sends one 9-byte **forward frame** every 20 ms (50 frames per second):

Full frame definitions are in [`docs/interface_control.md`](docs/interface_control.md).

## How it works

### Handheld: calculating the angles

Angles are calculated on the ESP32 before sending:

- **Tilt** comes from the accelerometer, using the direction of gravity from the Y and Z readings (`atan2`). Clamped to ±90° and sent as 0–180°.
- **Pan** comes from integrating the gyroscope's Z axis over time: `pan += (rate ÷ 131) × 0.02 s`, where 131 is the raw count per degree per second. Clamped to ±135° and sent as 0–270°.
- **Calibration:** at startup the handheld is held still for 2 seconds while 100 gyroscope readings are averaged. That offset is subtracted from every later reading.

### Handheld: timing

The send loop is a FreeRTOS task that uses `vTaskDelayUntil` to run every 20 ms, matching the servos' 50 Hz refresh rate. The fixed period is also what makes the `0.02 s` in the pan calculation correct. If the Bluetooth link reports congestion, the task skips that send.

### Controller: receiving frames state machine

The main loop reads the UART one byte at a time and feeds each byte to a four-state parser
* (first preamble byte → second preamble byte → type ID → payload). 

A complete frame is only accepted if its checksum matches. The onboard LED (PC13) toggles every 25 valid frames.

### Controller: driving the servos

* The STM32 runs at 16 MHz. 
* Timer 3's prescaler divides that by 16, so the timer ticks once per microsecond, and its period of 20,000 ticks gives a 20 ms (50 Hz) servo cycle. Because one tick is one microsecond, the pulse width in microseconds is written straight to the timer with `__HAL_TIM_SET_COMPARE`.

Each servo's pulse range was measured on the bench through the ST-LINK debugger in STM32CubeIDE:

| Servo | Range | Pulse width 
|---|---|---|
| Pan | 0–270° | 325–2600 µs
| Tilt | 0–180° | 500–2350 µs

Angles outside each range are clamped before conversion.

## Determinism

Design choices that keep timing and memory predictable:

- Fixed 9-byte frames
- Buffers sized at compile time, with no memory allocated at runtime
- A fixed 20 ms send period
- Frame checking with a preamble, type ID, and checksum
- Clamped angles and pulse widths

Known limitations:

- Bluetooth doesn't guarantee when, or whether, a frame arrives.
- The controller's loop timing depends on when bytes arrive.
- There is no link-loss timeout yet.
- Pan drifts slowly over time, because it comes from integrating the gyroscope and the MPU-6500 has no magnetometer to correct heading.

Bring-up details are in [`docs/BT_connection_setup.md`](docs/BT_connection_setup.md).

## Testing

- **Unit tests** for frame encoding and decoding, using `assert` ([`firmware/common/test/test_frame.c`](firmware/common/test/test_frame.c)): round trip, extreme values, and a corrupted type ID.
- **Link test:** the Bluetooth link was proven with Espressif's stock example before any custom code, so later failures could be traced to the code rather than the link.
- **End-to-end test:** ESP32 → HC-05 → STM32 → servos, sweeping commands across the servo range.
- **Next:** move the unit tests to a test framework such as Unity.

## Status

**Planned** (see [`docs/architecture.md`](docs/architecture.md)):
- FreeRTOS on the STM32
- A return frame from the controller to the handheld, and an OLED status display
- Watchdogs on both nodes
- Link-loss safe states: hold position after 200 ms without a frame, return the servos home after 500 ms

## My contributions vs Example Code

**Written by me:**
- Shared frame encoder and decoder, and its unit tests (`firmware/common/`)
- STM32 frame parser state machine (`frame_rx.c`) and angle-to-pulse-width conversion (`servo.c`)
- STM32 main loop and timer and UART configuration
- ESP32 send task: IMU reading, angle calculation, framing, and timing
- IMU startup and gyroscope calibration (`imu.c`)
- Documentation and hardware files

**Built on:**
- Espressif's `bt_spp_initiator` example (Bluetooth connection handling and serial console)
- Espressif's basic I2C example (register read and write helpers)
- STM32CubeMX-generated startup and peripheral setup code

## Repository layout

```
docs/          architecture, frame format, Bluetooth bring-up, toolchain, verification
firmware/
  common/      frame encoder and decoder shared by both nodes, plus unit tests
  esp32/       handheld node (ESP-IDF)
  stm32/       controller node (STM32CubeIDE / STM32CubeMX)
hardware/      parts list and KiCad schematics
```

## Building

Full toolchain details are in [`docs/deployment.md`](docs/deployment.md).

**Repository setup** (exports KiCad schematics to PDF on each commit):
```bash
git config core.hooksPath .githooks
```

**Handheld (ESP-IDF v6.1)**, from `firmware/esp32/`:
```bash
idf.py set-target esp32   # first time only
idf.py build
idf.py -p PORT flash monitor
```

**Controller:** open `firmware/stm32/` in STM32CubeIDE and flash over the ST-LINK.

**Unit tests**, from the repository root:
```bash
gcc -I firmware/common firmware/common/frame.c firmware/common/test/test_frame.c -o firmware/common/test_frame
./firmware/common/test_frame
```
