
#ifndef CONFIG_H
#define CONFIG_H

// ---------- Robot geometry (placeholder values, calibrate in week 3) ----------
#define WHEEL_RADIUS_M      0.033f   // wheel radius in meters
#define WHEEL_BASE_M        0.130f   // distance between wheel contact centers
#define ENCODER_PPR         20       // encoder pulses per wheel revolution

// ---------- Control loop ----------
#define CONTROL_DT_S        0.01f    // 100 Hz control period

// ---------- Pin assignments ----------
#define PIN_MOTOR_L_A       2
#define PIN_MOTOR_L_B       3
#define PIN_MOTOR_R_A       4
#define PIN_MOTOR_R_B       5
#define PIN_ENCODER_L       6
#define PIN_ENCODER_R       7
#define PIN_US_FRONT_TRIG   10
#define PIN_US_FRONT_ECHO   11
#define PIN_SERVO           15

#endif // CONFIG_H

