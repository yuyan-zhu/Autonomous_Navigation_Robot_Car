#include "pid.h"

void pid_reset(pid_ctrl_t *p) {
    p->integ = 0.0f;
    p->prev_err = 0.0f;
}

// Discrete PID: P + I + D, with clamping and anti-windup.
float pid_update(pid_ctrl_t *p, float setpoint, float measured, float dt) {
    if (dt <= 0.0f) {
        return 0.0f;
    }

    float err = setpoint - measured;
    float new_integ = p->integ + err * dt;
    float deriv = (err - p->prev_err) / dt;
    float u = p->kp * err + p->ki * new_integ + p->kd * deriv;

    // Anti-windup: freeze the integral while the output is saturated 
    // and the error would push it further into saturation.
    int saturating = (u > p->out_max && err > 0.0f) || (u < p->out_min && err < 0.0f);
    if (!saturating) {
        p->integ = new_integ;
    }

    if (u > p->out_max) {
        u = p->out_max;
    } else if (u < p->out_min) {
        u = p->out_min;
    }

    p->prev_err = err;
    return u;
}