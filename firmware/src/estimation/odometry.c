#include "odometry.h"

void odom_reset(void) {
}

void odom_update(int32_t d_left, int32_t d_right) {
    (void)d_left;
    (void)d_right;
}

pose_t odom_get_pose(void) {
    return (pose_t){0};
}