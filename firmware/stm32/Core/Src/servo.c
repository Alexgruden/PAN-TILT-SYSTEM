#include "servo.h"

/**
 * @brief Convert pan and tilt angles to servo pulse widths.
 * 
 * @param[in]  pan         Pan angle in degrees (0 to 270).
 * 
 * @return Pulse width in microseconds corresponding to the pan angle.
 */
int32_t pan_to_pulsewidth(int16_t pan) {
    // Clamp pan angle to 0-270 degrees
    if (pan < 0) {
        pan = 0;
    } else if (pan > 270)
    {
        pan = 270;
    }
    return PAN_MIN_PW + ((pan * 2275) / 270);
}

/**
 * @brief Convert pan and tilt angles to servo pulse widths.
 * 
 * @param[in]  tilt        Tilt angle in degrees (0 to 180).
 * 
 * @return Pulse width in microseconds corresponding to the tilt angle.
 */
int32_t tilt_to_pulsewidth(int16_t tilt) {
    // Clamp tilt angle to 0 to 180 degrees
    if (tilt < 0) {
        tilt = 0;
    } else if (tilt > 180) {
        tilt = 180;
    }
    return TILT_MIN_PW + ((tilt * 1850) / 180);
}