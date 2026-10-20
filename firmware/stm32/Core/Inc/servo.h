#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "frame.h"

#define PAN_MIN_PW 325
#define PAN_MAX_PW 2600
#define TILT_MIN_PW 500
#define TILT_MAX_PW 2350
#define PAN_CENTER 1460
#define TILT_CENTER 1450

/**
 * @brief Convert pan and tilt angles to servo pulse widths.
 * 
 * @param[in]  pan         Pan angle in degrees (-180 to 180).
 * 
 */
int32_t pan_to_pulsewidth(int16_t pan);

/**
 * @brief Convert pan and tilt angles to servo pulse widths.
 * 
 * @param[in]  tilt        Tilt angle in degrees (-90 to 90).
 * 
 */
int32_t tilt_to_pulsewidth(int16_t tilt);


#endif // SERVO_H