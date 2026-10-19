#include "frame_rx.h"

//  This file implements a state machine to parse incoming bytes into a ForwardFrame struct. It looks for the preamble, type ID, and payload, and checks the checksum before filling the frame struct.
static uint8_t buffer[FRAME_LEN];
static uint8_t count = 0;
static RxState state = RX_STATE_WAIT_PREAMBLE_1;

/**
 * @brief Parse a single byte of incoming data and update the frame and state machine.
 * 
 * This function implements a state machine to parse incoming bytes into a ForwardFrame struct. It looks for the preamble, type ID, and payload, and checks the checksum before filling the frame struct.
 * 
 * @param[in]  byte            The incoming byte to parse.
 * @param[out] frame           The ForwardFrame struct to fill when a complete frame is received.
 * @return true if a complete and valid frame was received, false otherwise.
 */
bool parse_rx_byte(uint8_t byte, ForwardFrame *frame) {
    // Update the state machine based on the incoming byte and current state
    switch (state)
    {
    // If we are waiting for the first preamble byte, check if the incoming byte matches. If it does, move to the next state and store the byte in the buffer.
    case RX_STATE_WAIT_PREAMBLE_1:
        if (byte == PREAMBLE_BYTE_1) {
            state = RX_STATE_WAIT_PREAMBLE_2;
            buffer[0] = byte;
            count = 1;
        }
        break;
    // If we are waiting for the second preamble byte, check if the incoming byte matches. 
    case RX_STATE_WAIT_PREAMBLE_2:
        if (byte == PREAMBLE_BYTE_2) {
            state = RX_STATE_WAIT_TYPE_ID;
            buffer[1] = byte;
            count++;
        } else if (byte == PREAMBLE_BYTE_1) {
            state = RX_STATE_WAIT_PREAMBLE_2;
            buffer[0] = byte;
            count = 1;
        } else {
            state = RX_STATE_WAIT_PREAMBLE_1;
            count = 0;
        }
        break;
    // If we are waiting for the type ID, check if the incoming byte matches the expected type ID. 
    case RX_STATE_WAIT_TYPE_ID:
        if (byte == TYPE_ID_FORWARD) {
            state = RX_STATE_WAIT_PAYLOAD;
            buffer[2] = byte;
            count++;
        } else {
            state = RX_STATE_WAIT_PREAMBLE_1;
            count = 0;
        }
        break;
    // If we are waiting for the payload, store incoming bytes in the buffer until we have received FRAME_LEN bytes. Once we have a complete frame, decode it.
    case RX_STATE_WAIT_PAYLOAD:
        buffer[count++] = byte;
        if (count < FRAME_LEN) {
            break;
        }
        if (frame_decode(buffer, frame)) {
            state = RX_STATE_WAIT_PREAMBLE_1;
            count = 0;
            return true;
        } else {
            state = RX_STATE_WAIT_PREAMBLE_1;
            count = 0;
            return false;
        }
        break;
    // Safety net fallback to the initial state if we somehow end up in an undefined state.
    default:
        state = RX_STATE_WAIT_PREAMBLE_1;
    }
    return false; // Return false if we haven't received a complete and valid frame yet.
}