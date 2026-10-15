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