#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdbool.h>

#define FRAME_LEN        9
#define PREAMBLE_BYTE_1  0xAAu
#define PREAMBLE_BYTE_2  0x55u
#define TYPE_ID_FORWARD  0x01u

/**
 * @brief Decoded contents of a forward frame (handheld -> controller).
 */
typedef struct {
    int16_t pan;
    int16_t tilt;
    uint8_t sequence;
} ForwardFrame;

/**
 * @brief Pack a forward frame into 9 bytes ready to send.
 *
 * Adds the preamble, type ID and checksum around the pan, tilt
 * and sequence values.
 *
 * @param[in]  frame  Pan, tilt and sequence values to send.
 * @param[out] out    Buffer of FRAME_LEN bytes to fill.
 */
void frame_encode(const ForwardFrame *frame, uint8_t out[FRAME_LEN]);


/**
 * @brief Unpack a forward frame from 9 received bytes.
 * 
 * Checks the preamble, type ID and checksum before unpacking the pan, tilt
 * and sequence values.
 * 
 * @param[in]  in     Buffer of FRAME_LEN bytes received.
 * @param[out] frame  Pan, tilt and sequence values to fill.
 * @return true if the frame was valid and unpacked, false otherwise.
 */
bool frame_decode(const uint8_t in[FRAME_LEN], ForwardFrame *frame);

#endif // FRAME_H