# Bluetooth Link Setup: ESP32 to HC-05

**Date:** 2026-10-17
**Result:** Pass. The ESP32 connected to the HC-05 and sent data continuously.

## Purpose

Prove that the ESP32 (handheld node) can find, pair with and connect to the HC-05 (controller node) over Bluetooth Classic serial port connection (SPP), before any custom code is written. If later code fails, this rules out the radio link.

## Setup

| Item | Details |
|---|---|
| Handheld | ESP32-WROOM-32 dev board, USB to laptop on `/dev/cu.usbserial-0001` |
| Controller | HC-05, powered from the Nucleo's +5V pin (CN6 pin 5) and GND. No STM32 code needed for this test. |
| Software | ESP-IDF v6.1 example `bt_spp_initiator`, copied to `~/esp/spp_test` (outside the repo) |
| Roles | ESP32 starts the connection. HC-05 stays in its default mode and waits to be connected to, so no AT commands were needed. |

## Configuration

These come from the example's `sdkconfig.defaults`:

| Setting | Value |
|---|---|
| Bluetooth controller mode | BR/EDR Only (Classic Bluetooth) |
| Bluedroid: Classic Bluetooth | Enabled |
| Bluedroid: SPP | Enabled |
| ESP BLE Mesh Support | **Disabled.** It conflicts with BR/EDR Only mode. |

## Changes Made to the Example

The HC-05 does not broadcast its name, so the example's match-by-name never fired. It was changed to match the HC-05 by address.
Note: The example code provided by ESP prints the address it found while scanning by default

**`main/main.c`, under `remote_device_name` (about line 46):**

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

The address `00:14:03:05:09:20` belongs to this specific HC-05. A different module needs its own address.

**Pairing PIN:** the example's `ESP_BT_GAP_PIN_REQ_EVT` handler sends `1234`, which matches the HC-05 default. No change needed.

## Procedure

```bash
source ~/.espressif/tools/activate_idf_v6.1.sh
cd ~/esp/spp_test
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

Exit the monitor with `Ctrl+]`. Press EN on the ESP32 to restart and watch from the beginning.

## Pass Criteria and Results

| Check | Expected | Result |
|---|---|---|
| HC-05 found during scan | `ESP_BT_GAP_DISC_RES_EVT` followed by `00 14 03 05 09 20` | Pass |
| Connection opened | `ESP_SPP_OPEN_EVT` in the monitor | Pass |
| HC-05 LED | Changes from fast blinking to a slow double blink | Pass |
| Throughput | At least 4.3 kbit/s, the combined forward and backward frame traffic | Pass: 5 to 10 kbit/s, about 8 kbit/s average |

The throughput limit comes from the HC-05's serial output at its default 9600 baud (about 7.7 kbit/s of data), not from Bluetooth. It can be raised with an AT command if ever needed.

## Problems Hit and Fixes

| Problem | Cause | Fix |
|---|---|---|
| Configuration warning about `BTDM_BLE_SCAN_DUPL` | ESP BLE Mesh was enabled, which needs BLE, while the controller was set to BR/EDR Only | Disabled ESP BLE Mesh Support in menuconfig |
| `could not open port PORT` | `PORT` was typed literally instead of the real port name | Found the port with `ls /dev/cu.*` |
| ESP32 not listed in `/dev/cu.*` | USB cable was charge-only | Swapped to a data cable |
| `idf.py: command not found` | New terminal had not been activated | Ran the activation script; `(venv)` in the prompt confirms it |
| ESP32 scanned but never connected | HC-05 does not broadcast its name | Matched by address instead (see Changes Made) |

## Next Step

Configure STM32 USART1 (PA9 transmit, PA10 receive) at 9600 baud and print received bytes to the laptop over the ST-LINK USB serial port.