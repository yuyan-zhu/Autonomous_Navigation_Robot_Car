#ifndef ODOMETRY_H
#define ODOMETRY_H

#include <stdint.h>

// Robot pose in the world frame.
// x, y in meters; theta in radians, always kept in (-pi, pi].
// theta = 0 points along +x; positive theta is a left (counter-clockwise) turn.
typedef struct {
    float x;
    float y;
    float theta;
} pose_t;

// Reset the pose to (0, 0, 0).
void odom_reset(void);

// Integrate one step of wheel motion.
// d_left / d_right: signed encoder pulse counts since the previous call
// (positive = wheel moving forward).
void odom_update(int32_t d_left, int32_t d_right);

// Return the current pose estimate.
pose_t odom_get_pose(void);

#endif // ODOMETRY_H
