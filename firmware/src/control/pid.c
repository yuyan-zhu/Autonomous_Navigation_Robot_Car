
#include "pid.h"

void pid_reset(pid_ctrl_t *p) {
    // TODO: clear integral and previous error
    (void)p;
}

float pid_update(pid_ctrl_t *p, float setpoint, float measured, float dt) {
    // TODO: implement P + I + D with clamping and anti-windup
    (void)p; (void)setpoint; (void)measured; (void)dt;
    return 0.0f;
}

