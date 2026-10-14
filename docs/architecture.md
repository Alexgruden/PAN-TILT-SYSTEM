# **Technical Specification: Bidirectional Fault-Tolerant Pan-Tilt Telemetry System**

## Node and link structure and data flow of the dual-RTOS system
- This system is designed to be bidirectional network with independent RTOS running on both sides.
- **Handheld node**
    - TX/RX Node (Transmit primary, Recieve secondary)
    - Uses ESP-IDF FreeRTOS

- **Controller node**
    - RX/TX Node (Recieve primary, Transmit secondary)
    - Uses STM32 FreeRTOS

### Visual Flow
```
+------------------------------------+                +--------------------------------------+

|        HANDHELD NODE (ESP32)       |                |        CONTROLLER NODE (STM32)       |
|                                    |                |                                      |
| [Sensor_Task] -> IMU Vectors       |  (Telemetry)   | [Parse_Task] -> Processes Vectors    |
|                                    | -------------> | [Servo_Task] -> Generates PWM Out    |
| [Display_Task] <- Updates UI Status|                |                                      |
| [Comms_Task]   <- Battery/Ack Logs | <------------- | [Heartbeat_Task] <- Telemetry Health |
|                                    |   (Feedback)   |                                      |
| +--------------------------------+ |                | +----------------------------------+ |
| | Watchdog Task & Bitmask Check  | |                | | Watchdog ISR & Register Checking | |
| +--------------------------------+ |                | +----------------------------------+ |
+------------------------------------+                +--------------------------------------+
```

## Comunication protocol and bidirectional flow

### Frame Matrix 
- This outlines the communication frames which will be sent from the ESP 32 to the HC-05 and back from the HC-05 to the ESP32
- The communication will be done using 9-byte frames to ensure as small of a payload over bluetooth as possible.
    - This will save bandwidth compaired to sending padded frames.

### Forward frame
- Sent every 20ms, this should match with the standard servo refresh rate (50hz)
```
+-----------+-----------+---------------+---------------+----------+---------+

| PREAMBLE  | TYPE ID   | PAN DATA (X)  | TILT DATA (Y) | SEQUENCE | CHKSUM  |
|  (2 Bytes)|  (1 Byte) |   (2 Bytes)   |   (2 Bytes)   |  (1 Byte)| (1 Byte)|
| 0xAA 0x55 |   0x01    |  int16_t MSB  |  int16_t MSB  |  uint8_t | XOR Val |
+-----------+-----------+---------------+---------------+----------+---------+
```

### Backward frame
- Sent every 100ms, this is slower since the OLED takes longer to refresh
```
+-----------+-----------+---------------+---------------+----------+---------+

| PREAMBLE  | TYPE ID   | SERVO STEPS X | SERVO STEPS Y | SYS_STAT | CHKSUM  |
|  (2 Bytes)|  (1 Byte) |   (2 Bytes)   |   (2 Bytes)   |  (1 Byte)| (1 Byte)|
| 0xAA 0x55 |   0x02    |  uint16_t MSB |  uint16_t MSB | Bitfield | XOR Val |
+-----------+-----------+---------------+---------------+----------+---------+
```