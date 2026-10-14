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

### Byte Definitions
**Forward Frame**:
- Byte 1 & Byte 2: The Handshake (0xAA and 0x55)
- Byte 3: The Message Type (0x01) - Allows for different message types later such as battery percentage etc.
- Byte 4 & Byte 5: The Pan Axis (X) - Allows for more flexibility with the angle commands
- Byte 6 & Byte 7: The Tilt Axis (Y) - same as pan axis
- Byte 8: The Sequence Counter - counts to allow for the RX to detect missing bytes.
- Byte 9: The Safety Check (Checksum) - XOR the whole frame and use the calculated XOR byte as a confirmation. Ensures Matching byte.

**Backward Frame**:
- Byte 1-7 and 9 are identical but respose from servos not instructions
- Byte 8: SYS_STAT - Instead of using the whole byte to indicate 1 status, the each bit is used to determine a single status
    - Examples could include: Bluetooth connection ok? 0 = no, 1 = yes, Servos responding? 0 = no, 1 = yes

## Watchdog for each node

### Handheld watchdog
- Software watchdog for task roll-call
- Uses EPS32 TWDT (Task watchdog timer) to ensure FreeRTOS tasks are successfully running
### Controller watchdog
- Hardware watchdog using the built in timer on the chip 
- Ignores individual software tasks and monitors if the entire chip is frozen, and triggers a reset accordingly.


## Software concurrency and task structure

### ESP 32 structure

```
                      +-------------------+

                       | FreeRTOS Sched    |
                       +-------------------+
                                 |
        +------------------------+------------------------+

        | (Core 0 - Comms Hub)                            | (Core 1 - Sensory & Processing)
        v                                                 v
+-----------------------+                                +-----------------------+
|                       | Inter-Process Communication   
| Comms_Task (Pri 3)    | <--- [IPC Queue] ------------- | Sensor_Task (Pri 4)   |
| Manage BT Frame IO    |                                | Read MPU9250 / Math   |
+-----------------------+                                +-----------------------+

        |                                                         |
        v                                                         v
+-----------------------+                                +-----------------------+

| WDT_Monitor (Pri 5)   |                                | Display_Task (Pri 1)  |
| Bitmask Verification  |                                | Updates OLED Screens  |
+-----------------------+                                +-----------------------+
```

### STM 32 Structure 
- **Telemetry_Parse_Task** (Priority: High / Period: Event-Driven): Blocked until unblocked by the USART1 
- **Servo_Control_Task** (Priority: Above Normal / Period: 20 ms): Command the physical servos
- **Heartbeat_Broadcast_Task** (Priority: Normal / Period: 100 ms): pushes the tracking state frame back through bluetooth to the handheld node.

## Recovery System Outline

```
       +---------------------------------------+

       |         NORMAL RUNTIME STATE          |
       +---------------------------------------+
                           |
                           | [Condition: No Frame for > 200ms]
                           v
       +---------------------------------------+

       |         LINK_DEGRADED STATE             |
       |  - Freeze servos at last known position |
       |  - Start incremental retry timeout      |
       +---------------------------------------+
                           |
                           | [Condition: Timeout Reaches 500ms]
                           v
       +---------------------------------------+

       |         CRITICAL_SAFE STATE           |
       |  - Actively home servos to default    |
       |  - Drop Bluetooth                     |
       |  - Flash visual LCD warning           |
       +---------------------------------------+
```