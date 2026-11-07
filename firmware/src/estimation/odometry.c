#include "odometry.h"

#include <math.h>
#include "config.h"

#define PI_F 3.14159265f
#define M_PER_PULSE (2.0f * PI_F * WHEEL_RADIUS_M / (float)ENCODER_PPR)

static pose_t s_pose;

void odom_reset(void) {
    s_pose.x = 0.0f;
    s_pose.y = 0.0f;
    s_pose.theta = 0.0f;
}

void odom_update(int32_t d_left, int32_t d_right) {
    float d_l = d_left * M_PER_PULSE;
    float d_r = d_right * M_PER_PULSE;
    float d = (d_l + d_r) / 2.0f;
    s_pose.x += d;
}

pose_t odom_get_pose(void) {
    return s_pose;
}