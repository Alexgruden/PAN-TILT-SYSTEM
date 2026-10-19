# Bluetooth Link Bring-Up: ESP32 to STM32

**Result:** Pass. Bytes sent by the ESP32 reach the STM32 through the HC-05.

| Test | Date | Result |
|---|---|---|
| 1. ESP32 connects to the HC-05 and sends data | 2026-10-17 | Pass |
| 2. STM32 receives that data over USART1 | 2026-10-18 | Pass |

**Purpose:** prove the radio link works before writing custom code, so later failures can be traced to the code rather than the link.

## Setup

| Item | Details |
|---|---|
| Handheld | ESP32-WROOM-32 dev board, on `/dev/cu.usbserial-0001` |
| Controller | STM32F411CEU6 board, flashed from STM32CubeIDE with an ST-LINK V2 dongle over SWD |
| Bluetooth roles | The ESP32 starts the connection. The HC-05 stays in its default mode, so no AT commands are needed. |
| ESP32 software | ESP-IDF v6.1 example `bt_spp_initiator`, in `~/esp/spp_test` |
| STM32 software | USART1 at 9600 baud. Toggles the PC13 LED every 100 bytes received. |

**HC-05 wiring to the STM32 board:**

| HC-05 | STM32 board |
|---|---|
| VCC | 5V |
| GND | G |
| TXD | A10 (PA10, USART1 receive) |
| RXD | A9 (PA9, USART1 transmit) |

## ESP32 Configuration

From the example's `sdkconfig.defaults`, plus one change in menuconfig:

| Setting | Value |
|---|---|
| Bluetooth controller mode | BR/EDR Only (Classic Bluetooth) |
| Bluedroid: Classic Bluetooth, SPP | Enabled |
| ESP BLE Mesh Support | **Disabled.** It conflicts with BR/EDR Only mode. |

## Changes to the Example

The HC-05 doesn't broadcast its name, so the example's match-by-name never fires. It now matches the HC-05 by address instead. The example prints the address of every device it finds while scanning, which is how this address was found.

**`main/main.c`, under `remote_device_name`:**

```c
static esp_bd_addr_t hc05_addr = {0x00, 0x14, 0x03, 0x05, 0x09, 0x20};
```

**`main/main.c`, in `case ESP_BT_GAP_DISC_RES_EVT:`, after `ESP_LOG_BUFFER_HEX(...)`:**

```c
if (memcmp(param->disc_res.bda, hc05_addr, ESP_BD_ADDR_LEN) == 0) {
    memcpy(peer_bd_addr, param->disc_res.bda, ESP_BD_ADDR_LEN);
    esp_bt_gap_cancel_discovery();
    esp_spp_start_discovery(peer_bd_addr);
}
```

This address belongs to this specific HC-05. A different module needs its own. The pairing PIN, `1234`, is the HC-05 default and needed no change.

## Running the Test

```bash
source ~/.espressif/tools/activate_idf_v6.1.sh
cd ~/esp/spp_test
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

Exit the monitor with `Ctrl+]`. Press EN on the ESP32 to restart.

## Results

| Check | Expected | Result |
|---|---|---|
| HC-05 found | `ESP_BT_GAP_DISC_RES_EVT` followed by `00 14 03 05 09 20` | Pass |
| Connection opened | `ESP_SPP_OPEN_EVT`, and the HC-05 LED changes to a slow double blink | Pass |
| Throughput | At least 4.3 kbit/s, the combined frame traffic | Pass: 5 to 10 kbit/s, about 8 average |
| STM32 receives | PC13 LED blinks while the ESP32 sends, and stops when it's off | Pass |

Throughput is limited by the HC-05's 9600 baud serial output, not by Bluetooth. It can be raised with an AT command if ever needed.

## Problems and Fixes

| Problem | Fix |
|---|---|
| `BTDM_BLE_SCAN_DUPL` configuration warning | Disabled ESP BLE Mesh Support, which needs BLE |
| ESP32 not listed in `/dev/cu.*` | Swapped a charge-only USB cable for a data cable |
| `idf.py: command not found` | Ran the activation script. `(venv)` in the prompt confirms it. |
| ESP32 scanned but never connected | Matched the HC-05 by address instead of by name |
| The original NUCLEO-F401RE's ST-LINK couldn't reach its STM32, with a supply reading near 5V | Replaced it with the STM32F411CEU6 board and an external ST-LINK dongle |

## Next Step

The ESP32 sends real forward frames (`frame_encode`) every 20 ms. The STM32 decodes them (`frame_decode`) and toggles the LED only on valid frames.