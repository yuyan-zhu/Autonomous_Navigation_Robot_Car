
#ifndef PID_H
#define PID_H

// Discrete PID controller with output clamping and anti-windup.
// Hardware-independent: must not include any Pico SDK headers.
typedef struct {
    float kp, ki, kd;
    float integ;      // accumulated integral of error
    float prev_err;   // error from the previous update
    float out_min, out_max;
} pid_ctrl_t;

void  pid_reset(pid_ctrl_t *p);
float pid_update(pid_ctrl_t *p, float setpoint, float measured, float dt);

#endif // PID_H

