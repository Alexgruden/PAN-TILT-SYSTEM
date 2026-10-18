#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"


/**
 * @brief Test the frame encoding and decoding functions.
 * 
 * This function encodes a ForwardFrame struct into a byte buffer, then decodes it back into a ForwardFrame struct and checks that the values match.
 */
int main() {
    //Initialize trackers
    uint8_t buffer[FRAME_LEN] = {0};

    // Encode a frame with pan=300, tilt=-45, sequence=0
    ForwardFrame f;
    f.pan = 300;
    f.tilt = -45;
    f.sequence = 5;
    frame_encode(&f, buffer);   // writes all 9 bytes of buf

    // Decode the frame back into a ForwardFrame struct
    ForwardFrame decoded_frame;
    bool decode_result = frame_decode(buffer, &decoded_frame);
    assert(decode_result);  // should return true
    printf("Received forward frame, roundtrip successful\n");
    printf("Recieved Known frame, Decoded frame : pan=%d, tilt=%d, sequence=%d, Known answer successful\n", decoded_frame.pan, decoded_frame.tilt, decoded_frame.sequence);

    // Encode a frame with pan=0, tilt=-0, sequence=0
    memset(buffer, 0, FRAME_LEN);
    ForwardFrame f2;
    f2.pan = 0;
    f2.tilt = 0;
    f2.sequence = 0;
    frame_encode(&f2, buffer);   // writes all 9 bytes of buf

    // Decode the frame back into a ForwardFrame struct
    ForwardFrame decoded_frame2;
    bool decode_result2 = frame_decode(buffer, &decoded_frame2);
    assert(decode_result2);  // should return true
    printf("Received forward frame2, roundtrip successful\n");
    printf("Recieved Known frame2, Decoded frame : pan=%d, tilt=%d, sequence=%d, Known answer successful\n", decoded_frame2.pan, decoded_frame2.tilt, decoded_frame2.sequence);

    // Encode a frame with pan=32767, tilt=-32767, sequence=255
    memset(buffer, 0, FRAME_LEN);
    ForwardFrame f3;
    f3.pan = 32767;
    f3.tilt = -32768; // 
    f3.sequence = 255;
    frame_encode(&f3, buffer);   // writes all 9 bytes of buf

    // Decode the frame back into a ForwardFrame struct
    ForwardFrame decoded_frame3;
    bool decode_result3 = frame_decode(buffer, &decoded_frame3);
    assert(decode_result3);  // should return true
    printf("Received forward frame3, roundtrip successful\n");
    printf("Recieved Known frame3, Decoded frame : pan=%d, tilt=%d, sequence=%d, Known answer successful\n", decoded_frame3.pan, decoded_frame3.tilt, decoded_frame3.sequence);
}
