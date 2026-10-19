#include <frame.h>

// Assign the preamble, type ID, pan, tilt, sequence and checksum to the output buffer.
void frame_encode(const ForwardFrame *frame, uint8_t out[FRAME_LEN]){
    out[0] = PREAMBLE_BYTE_1;
    out[1] = PREAMBLE_BYTE_2;
    out[2] = TYPE_ID_FORWARD;
    out[3] = (frame->pan >> 8) & 0xFF;    // pan, top byte
    out[4] = frame->pan & 0xFF;           // pan, bottom byte
    out[5] = (frame->tilt >> 8) & 0xFF;   // tilt, top byte
    out[6] = frame->tilt & 0xFF;          // tilt, bottom byte
    out[7] = frame->sequence;
    out[8] = out[0] ^ out[1] ^ out[2] ^ out[3] ^ out[4] ^ out[5] ^ out[6] ^ out[7]; // checksum
};

bool frame_decode(const uint8_t in[FRAME_LEN], ForwardFrame *frame) {
    // Check preamble
    if (in[0] != PREAMBLE_BYTE_1 || in[1] != PREAMBLE_BYTE_2) {
        return false;
    }

    // Check type ID
    if (in[2] != TYPE_ID_FORWARD) {
        return false;
    }

    // Check checksum
    uint8_t checksum = in[0] ^ in[1] ^ in[2] ^ in[3] ^ in[4] ^ in[5] ^ in[6] ^ in[7];
    if (checksum != in[8]) {
        return false;
    }

    // Unpack pan, tilt and sequence
    frame->pan = (int16_t)((in[3] << 8) | in[4]);
    frame->tilt = (int16_t)((in[5] << 8) | in[6]);
    frame->sequence = in[7];

    return true;
}; 