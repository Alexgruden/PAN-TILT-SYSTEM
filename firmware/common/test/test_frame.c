#include <assert.h>
#include <stdio.h>
#include "frame.h"


/**
 * @brief Test the frame encoding and decoding functions.
 * 
 * This function encodes a ForwardFrame struct into a byte buffer, then decodes it back into a ForwardFrame struct and checks that the values match.
 */
int main() {
    //Initialize trackers
    int seq = 0;
    uint8_t buffer[FRAME_LEN];

    // Encode a frame with pan=300, tilt=-45, sequence=0
    ForwardFrame f;
    f.pan = 300;
    f.tilt = -45;
    f.sequence = seq++;
    frame_encode(&f, buffer);   // writes all 9 bytes of buf

    // Decode the frame back into a ForwardFrame struct
    ForwardFrame decoded_frame;
    bool decode_result = frame_decode(buffer, &decoded_frame);
    assert(decode_result);  // should return true
    printf("Decoded frame: pan=%d, tilt=%d, sequence=%d\n", decoded_frame.pan, decoded_frame.tilt, decoded_frame.sequence);
}
