#ifndef FRAME_RX_H
#define FRAME_RX_H

#include <stdint.h>
#include <stdbool.h>
#include "frame.h"

/**
 * @brief States for the frame receiver state machine.
 */
typedef enum {
    RX_STATE_WAIT_PREAMBLE_1,
    RX_STATE_WAIT_PREAMBLE_2,
    RX_STATE_WAIT_TYPE_ID,
    RX_STATE_WAIT_PAYLOAD,
} RxState;

/** 
 * @brief Parse a single byte of incoming data and update the frame and state machine.
 * 
 * @param[in]  byte            The incoming byte to parse.
 * @param[out] frame           The ForwardFrame struct to fill when a complete frame is received.
 */
bool parse_rx_byte(uint8_t byte, ForwardFrame *frame);

#endif // FRAME_RX_H