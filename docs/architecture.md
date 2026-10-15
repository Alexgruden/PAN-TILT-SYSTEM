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