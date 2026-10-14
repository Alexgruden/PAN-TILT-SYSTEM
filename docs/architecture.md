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