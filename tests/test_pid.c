
#include <stdio.h>
#include <math.h>
#include "pid.h"

static int failures = 0;

#define ASSERT_NEAR(a, b, tol) do { \
    if (fabsf((a) - (b)) > (tol)) { \
        printf("FAIL %s:%d: %f != %f\n", __FILE__, __LINE__, (double)(a), (double)(b)); \
        failures++; } } while (0)

#define ASSERT_TRUE(c) do { if (!(c)) { \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

// Build a PID with symmetric output limits [-lim, lim].
static pid_ctrl_t make(float kp, float ki, float kd, float lim) {
    pid_ctrl_t p = { kp, ki, kd, 0, 0, -lim, lim };
    pid_reset(&p);
    return p;
}

static void test_p_only(void) {
    pid_ctrl_t p = make(1.0f, 0, 0, 1.0f);
    ASSERT_NEAR(pid_update(&p, 1.0f, 0.5f, 0.01f), 0.5f, 1e-6f);
}

static void test_output_clamped(void) {
    pid_ctrl_t p = make(10.0f, 0, 0, 1.0f);
    ASSERT_NEAR(pid_update(&p,  1.0f, 0.0f, 0.01f),  1.0f, 1e-6f);
    ASSERT_NEAR(pid_update(&p, -1.0f, 0.0f, 0.01f), -1.0f, 1e-6f);
}

static void test_integral_accumulates(void) {
    pid_ctrl_t p = make(0, 1.0f, 0, 10.0f);
    float u = 0;
    for (int i = 0; i < 100; i++) u = pid_update(&p, 1.0f, 0.0f, 0.01f);
    ASSERT_NEAR(u, 1.0f, 1e-3f);   // 100 steps * 0.01 s * error 1 = 1.0
}

static void test_anti_windup(void) {
    // Saturate for 10 s, then reverse the error.
    pid_ctrl_t p = make(0, 1.0f, 0, 0.5f);
    for (int i = 0; i < 1000; i++) pid_update(&p, 1.0f, 0.0f, 0.01f);
    float u = 0;
    for (int i = 0; i < 60; i++) u = pid_update(&p, 0.0f, 1.0f, 0.01f);
    // With anti-windup the output recovers within ~0.6 s.
    // Without it, the integral would be ~10 and take ~10 s to unwind.
    ASSERT_TRUE(u < 0.0f);
}

int main(void) {
    test_p_only();
    test_output_clamped();
    test_integral_accumulates();
    test_anti_windup();
    if (failures) printf("%d check(s) failed\n", failures);
    else          printf("All PID tests passed\n");
    return failures != 0;
}






