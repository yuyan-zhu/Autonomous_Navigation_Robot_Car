// Host-side unit tests for the differential-drive odometry module.
// Build and run from the repository root with: make test

#include <math.h>
#include <stdio.h>

#include "config.h"
#include "odometry.h"

static int failures = 0;

#define ASSERT_NEAR(actual, expected, tol)                                   \
    do {                                                                     \
        float a_ = (actual), e_ = (expected);                                \
        if (fabsf(a_ - e_) > (tol)) {                                        \
            printf("FAIL %s:%d: %s = %f, expected %f\n", __FILE__, __LINE__, \
                   #actual, a_, e_);                                         \
            failures++;                                                      \
        }                                                                    \
    } while (0)

#define ASSERT_TRUE(cond)                                                    \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            failures++;                                                      \
        }                                                                    \
    } while (0)

#define TEST_PI 3.14159265f
// Distance one wheel travels per encoder pulse, in meters.
#define M_PER_PULSE (2.0f * TEST_PI * WHEEL_RADIUS_M / (float)ENCODER_PPR)

// After reset the pose must be exactly the origin.
static void test_reset(void) {
    odom_reset();
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, 0.0f, 1e-6f);
    ASSERT_NEAR(p.y, 0.0f, 1e-6f);
    ASSERT_NEAR(p.theta, 0.0f, 1e-6f);
}

// Equal pulses on both wheels: straight line along +x, heading unchanged.
// One full wheel revolution moves the robot by one wheel circumference.
static void test_straight_one_revolution(void) {
    odom_reset();
    odom_update(ENCODER_PPR, ENCODER_PPR);
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, 2.0f * TEST_PI * WHEEL_RADIUS_M, 1e-5f);
    ASSERT_NEAR(p.y, 0.0f, 1e-6f);
    ASSERT_NEAR(p.theta, 0.0f, 1e-6f);
}

// Many small steps (like the real 100 Hz loop) must add up to the same distance.
static void test_straight_many_small_steps(void) {
    odom_reset();
    for (int i = 0; i < 100; i++) {
        odom_update(1, 1);
    }
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, 100.0f * M_PER_PULSE, 1e-4f);
    ASSERT_NEAR(p.y, 0.0f, 1e-6f);
}

// Opposite pulses: rotate in place. Position stays put, heading changes by
// (d_R - d_L) / b. Right wheel forward = left (positive) turn.
static void test_rotate_in_place(void) {
    odom_reset();
    odom_update(-5, 5);
    pose_t p = odom_get_pose();
    float expected_dtheta = (2.0f * 5.0f * M_PER_PULSE) / WHEEL_BASE_M;
    ASSERT_NEAR(p.x, 0.0f, 1e-6f);
    ASSERT_NEAR(p.y, 0.0f, 1e-6f);
    ASSERT_NEAR(p.theta, expected_dtheta, 1e-5f);
    ASSERT_TRUE(p.theta > 0.0f);
}

// One arc step: the position update must use the midpoint heading
// theta + dtheta / 2, not the old heading. With the old heading y would stay 0.
static void test_arc_uses_midpoint_heading(void) {
    odom_reset();
    odom_update(10, 20);
    float d_l = 10.0f * M_PER_PULSE;
    float d_r = 20.0f * M_PER_PULSE;
    float d = 0.5f * (d_l + d_r);
    float dtheta = (d_r - d_l) / WHEEL_BASE_M;
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, d * cosf(0.5f * dtheta), 1e-5f);
    ASSERT_NEAR(p.y, d * sinf(0.5f * dtheta), 1e-5f);
    ASSERT_NEAR(p.theta, dtheta, 1e-5f);
}

// Turn left first, then drive straight: the robot must move along its new heading.
static void test_turn_then_straight(void) {
    odom_reset();
    odom_update(-5, 5);
    float theta0 = odom_get_pose().theta;
    odom_update(ENCODER_PPR, ENCODER_PPR);
    float dist = (float)ENCODER_PPR * M_PER_PULSE;
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, dist * cosf(theta0), 1e-5f);
    ASSERT_NEAR(p.y, dist * sinf(theta0), 1e-5f);
    ASSERT_TRUE(p.y > 0.0f);
}

// Spinning many times in either direction: theta must always stay in (-pi, pi],
// and the robot must not drift away from the origin.
static void test_theta_normalization(void) {
    odom_reset();
    for (int i = 0; i < 1000; i++) {
        odom_update(-5, 5);
        float th = odom_get_pose().theta;
        if (!(th > -TEST_PI - 1e-5f && th <= TEST_PI + 1e-5f)) {
            printf("FAIL %s:%d: theta = %f out of range at step %d (left spin)\n",
                   __FILE__, __LINE__, th, i);
            failures++;
            break;
        }
    }
    odom_reset();
    for (int i = 0; i < 1000; i++) {
        odom_update(5, -5);
        float th = odom_get_pose().theta;
        if (!(th > -TEST_PI - 1e-5f && th <= TEST_PI + 1e-5f)) {
            printf("FAIL %s:%d: theta = %f out of range at step %d (right spin)\n",
                   __FILE__, __LINE__, th, i);
            failures++;
            break;
        }
    }
    pose_t p = odom_get_pose();
    ASSERT_NEAR(p.x, 0.0f, 1e-4f);
    ASSERT_NEAR(p.y, 0.0f, 1e-4f);
}

int main(void) {
    test_reset();
    test_straight_one_revolution();
    test_straight_many_small_steps();
    test_rotate_in_place();
    test_arc_uses_midpoint_heading();
    test_turn_then_straight();
    test_theta_normalization();

    if (failures) {
        printf("%d odometry test(s) failed\n", failures);
    } else {
        printf("All odometry tests passed\n");
    }
    return failures != 0;
}
