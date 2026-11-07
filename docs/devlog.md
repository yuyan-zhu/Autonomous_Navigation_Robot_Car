# Dev Log

## 2026-11-01

**Done**
- Installed VS Code + Raspberry Pi Pico extension; blink + USB serial (`hello N`) working
- Set up the project directory layout; moved `main.c` into `firmware/src/`
- Created GitHub repo and pushed the first commit

**Problem → Cause → Fix**
- Compile failed after deleting `build/` ("No such file or directory")
  → the Compile button only runs ninja; it doesn't recreate the build folder
  → ran "Raspberry Pi Pico: Configure CMake", then compiled again
- After moving `main.c` into `src/`, the build would break
  → CMakeLists.txt still pointed to the old path
  → changed to `add_executable(firmware src/main.c)` and added `include/` to include dirs
- Commits were not linked to my GitHub account (gray avatar)
  → I typed the email with Chinese curly quotes “ ”, so they became part of the email
  → reset `user.email` without quotes, then `git commit --amend --reset-author` and `git push --force`

**Learned**
- Always use the English keyboard in the terminal; Chinese quotes look similar but are different characters
- Git basics: add → commit → push

**Next**
- Step 2: hardware inventory (pins, encoder type, measurements) → `docs/hardware.md`

## 2026-11-05 - Hardware survey (week 1, step 2)

**Done**
- Identified the L9110S pinout from the silkscreen. Header order is `B-1A, B-2A, GND, VCC, A-1A, A-1B`; channel B uses `1A/2A` while channel A uses `1A/1B` (inconsistent labeling, same meaning).
- Assigned channel A (GP2/GP3) to the left motor and channel B (GP4/GP5) to the right motor. Note that MOTOR A terminals are on the right side of the board.
- Confirmed the motors are plain 2-wire TT gear motors with no built-in Hall encoder.
- Found 20-slot code discs already mounted on the motor shafts, but the kit has no slot-type sensors to read them. The encoder is therefore single-phase optical.
- Measured wheel circumference (211 mm, jumper wire wrapped around the tire), giving a diameter of 67.2 mm. Measured track width: 129 mm.
- Documented everything in `docs/hardware.md`.

**Problems / decisions**
- No encoder sensors in the kit: step 6 (encoder counting) and experiment A are blocked until LM393 slot sensors arrive. Steps 3-5 and experiment B can go ahead.
- Wheel circumference was measured with a wrapped wire, which likely overestimates it (67.2 mm vs 65 mm nominal, about 3%). This will be corrected by calibration in week 3.
- Power plan: 6 x AA NiMH (7.2 V) for the motors, LM2596 buck converter for a 5 V rail (servo, ultrasonic, later Pico VSYS via a Schottky diode). The Pico stays on USB power this week.
- HC-SR04 is the 5 V version, so ECHO needs a 1 kΩ / 2 kΩ divider before connecting to a GPIO.

**Next**
- Motor resistance and battery voltage (needs a multimeter).
- Step 3: power wiring. Step 4: motor driver code.


## 2026-11-06 — Project skeleton, config, and PID module (host-tested)

### Summary

While waiting for hardware (multimeter, LM2596 buck converter, battery, capacitors, LM393 slot
sensors), I worked on the hardware-independent parts of the firmware. By the end of the day:

- The repository has its final directory layout, and the Pico firmware builds with the new structure.
- All robot constants live in one header, `config.h`.
- The PID controller is fully implemented in plain C, with output clamping and anti-windup.
- The PID is verified by unit tests that run on my Mac (`make test`), not on the Pico.
- The same `pid.c` also compiles into the Pico firmware with 0 errors and 0 warnings.

---

### 1. Repository layout

```
Autonomous_Navigation_Robot_Car/
├── firmware/                  # C code that runs on the Pico 2 (built with CMake + Pico SDK)
│   ├── CMakeLists.txt
│   ├── include/config.h       # all constants: geometry, control rate, pin numbers
│   └── src/
│       ├── main.c
│       ├── drivers/           # motor, encoder, ultrasonic, servo (empty for now)
│       ├── control/           # pid.c / pid.h  ← implemented today
│       ├── estimation/        # odometry, grid map (next)
│       ├── planning/          # A*, navigation state machine
│       └── comm/              # telemetry
├── tests/                     # unit tests compiled and run on the host computer
│   └── test_pid.c
├── tools/                     # Python tools (plotting, simulator) — later
├── docs/                      # devlog, hardware notes, results
└── Makefile                   # builds and runs host-side tests
```

**Why this layout:**

- The `src/` subfolders mirror the four software layers (drivers → control → estimation →
  planning). Each layer only calls the layer below it, so code written early (e.g. the PID)
  does not need to change when higher layers are added.
- Firmware, host tests, and Python tools are separated so that each build system only sees
  the files it needs. CMake never touches `tests/`, and the test Makefile never touches the
  Pico SDK.

---

### 2. Firmware build configuration (CMake)

`CMakeLists.txt` is the build description for the Pico firmware. It is not C code — it tells
CMake which source files to compile, where to find headers, and which SDK libraries to link.
CMake turns it into Ninja build files, Ninja calls the ARM compiler for each `.c` file, and the
linker produces `firmware.elf` / `firmware.uf2`.

Changes made today (the `src/main.c` path and `include/` were set up on 2026-11-01; today added
`src/control/pid.c` and the `src/control` and `src/estimation` include paths):

```cmake
add_executable(firmware src/main.c src/control/pid.c)

target_include_directories(firmware PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/src/control
        ${CMAKE_CURRENT_LIST_DIR}/src/estimation
)
```

**Rule of thumb for when `CMakeLists.txt` must be edited:**

| Situation | What to change |
|---|---|
| A new `.c` file is created | Add it to `add_executable` |
| Headers live in a new folder | Add the folder to `target_include_directories` |
| A new SDK feature is used (e.g. PWM, I2C) | Add the library (e.g. `hardware_pwm`) to `target_link_libraries` |
| Only code inside existing files changes | Nothing |

Note: a source file must exist before it is listed in `add_executable`, otherwise CMake fails
with "Cannot find source file". Include directories can be added before they contain files.

---

### 3. `config.h` — single source of truth for constants

```c
#define WHEEL_RADIUS_M      0.033f   // placeholder, calibrate in week 3
#define WHEEL_BASE_M        0.130f   // placeholder, calibrate in week 3
#define ENCODER_PPR         20       // placeholder, verify with hardware
#define CONTROL_DT_S        0.01f    // 100 Hz control loop
#define PIN_MOTOR_L_A       2
// ... other pin assignments
```

**Design decisions:**

- **No magic numbers.** Every physical constant and pin number is defined once. When the wheel
  radius is calibrated later, one line changes instead of several files.
- **No Pico SDK includes.** `config.h` contains only `#define`s, so it can be included by
  hardware-independent modules that are also compiled on the host for testing.
- **Units in names.** `_M` = meters, `_S` = seconds, `_RADPS` = rad/s. This prevents mixing up
  centimeters and meters.
- **`f` suffix on literals.** `0.033f` is a `float`; `0.033` would be a `double`. The RP2350
  (Cortex-M33) has a single-precision FPU, so `float` math is done in hardware while `double`
  math is emulated in software and is several times slower.
- **Include guard** (`#ifndef CONFIG_H ... #endif`) prevents redefinition errors if the header
  is included more than once.

I verified the include path by adding `#include "config.h"` to `main.c` and rebuilding.

---

### 4. PID controller (`firmware/src/control/pid.c`)

#### 4.1 What it does in this robot

Two motors given the same PWM do not spin at the same speed (motor variation, battery voltage
drop, floor friction), so an open-loop robot drifts sideways. The PID closes the loop on
**wheel speed**: every 10 ms it compares the target speed with the speed measured from the
encoder and adjusts the PWM duty cycle. One PID instance runs per wheel.

```
target speed ──►(+)── error ──► PID ──► PWM duty ──► motor ──► wheel
                 ▲(−)                                           │
                 └────────────── measured speed ◄── encoder ◄───┘
```

The three terms:

| Term | Based on | Role |
|---|---|---|
| **P** (proportional) | current error | main corrective action |
| **I** (integral) | accumulated past error | removes steady-state error (e.g. constant friction) |
| **D** (derivative) | rate of change of error | damps overshoot and oscillation |

#### 4.2 Interface (`pid.h`)

```c
typedef struct {
    float kp, ki, kd;
    float integ;            // accumulated integral of error
    float prev_err;         // error from the previous update
    float out_min, out_max; // output limits
} pid_ctrl_t;

void  pid_reset(pid_ctrl_t *p);
float pid_update(pid_ctrl_t *p, float setpoint, float measured, float dt);
```

**Design decisions:**

- **State lives in a struct, passed by pointer.** The left and right wheels each get their own
  `pid_ctrl_t`, sharing the same code without sharing state. A pointer is required because
  `pid_update` must modify `integ` and `prev_err` in the caller's struct.
- **Gains and limits are set by the caller, not hard-coded in `pid.c`.** The same controller can
  be reused elsewhere (e.g. a heading controller with limits of ±3 rad/s).
- **Hardware-independent.** `pid.c` includes no Pico SDK headers, so it compiles both for the
  Pico and on the host for unit testing.
- **`pid_reset`** clears the integral and previous error. It should be called whenever the robot
  stops or the target changes abruptly, so that old integral does not cause a jump.

#### 4.3 Algorithm

Discrete form, evaluated once per control period $\Delta t$:

$$
e_k = r_k - y_k
$$

$$
I_k = I_{k-1} + e_k \, \Delta t
$$

$$
D_k = \frac{e_k - e_{k-1}}{\Delta t}
$$

$$
u_k = K_p e_k + K_i I_k + K_d D_k
$$

Order of operations inside `pid_update`:

1. **Guard:** if `dt <= 0`, return 0 (prevents division by zero in the derivative).
2. Compute the error `err = setpoint - measured`.
3. Compute a **candidate** integral `new_integ = integ + err * dt` (not stored yet).
4. Compute the derivative `(err - prev_err) / dt`.
5. Compute the unclamped output `u = kp*err + ki*new_integ + kd*deriv`.
6. **Anti-windup:** store `new_integ` only if the controller is *not* saturating (see 4.4).
7. **Clamp** `u` to `[out_min, out_max]`.
8. Store `prev_err = err` and return `u`.

Steps 6 and 7 must happen in this order: the saturation check needs the *unclamped* `u`.
After clamping, `u` is never out of range, so the check would always be false.

#### 4.4 Anti-windup (conditional integration)

**The problem (integral windup):** if the wheel is blocked, the error stays large, the output
hits its limit (duty = 1.0), but the integral keeps growing in the background. When the wheel
is released, the oversized integral keeps the output saturated for a long time and the robot
lurches forward.

**The fix:** freeze the integral when it would only push the output further into saturation:

```c
int saturating = (u > p->out_max && err > 0.0f) ||
                 (u < p->out_min && err < 0.0f);
if (!saturating) {
    p->integ = new_integ;
}
```

The error sign matters. If the output is at the upper limit but the error has turned negative,
the integral *should* be updated, because decreasing it is exactly what brings the output back
out of saturation.

Clamping and anti-windup are two separate protections:

- **Anti-windup** protects the controller's internal state (the integral).
- **Clamping** protects the actuator (the motor only accepts duty in [−1, 1]).

---

### 5. Host-side unit tests

#### 5.1 Approach: test-driven development (red → green)

1. Wrote `pid.h` and a **stub** `pid.c` (correct function signatures, no logic, always returns 0)
   so that everything compiles.
2. Wrote the tests and confirmed they **all failed** against the stub (5 of 5 checks failed).
   This proves the tests can actually detect a wrong implementation.
3. Implemented the PID step by step, re-running the tests after each step, until all passed.

#### 5.2 Test cases (`tests/test_pid.c`)

| Test | Setup | Expected | What it verifies |
|---|---|---|---|
| `test_p_only` | Kp = 1, error = 0.5 | output = 0.5 | proportional term |
| `test_output_clamped` | Kp = 10, error = ±1, limit = ±1 | output = ±1 | output clamping |
| `test_integral_accumulates` | Ki = 1, error = 1 for 100 steps of 0.01 s | output ≈ 1.0 | integral = Σ e·Δt |
| `test_anti_windup` | Ki = 1, limit = 0.5; saturate for 10 s, then reverse error for 0.6 s | output < 0 | integral does not wind up |

How the anti-windup test discriminates:

- **Without anti-windup**, the integral grows to ≈ 10 during the 10 s of saturation. Reversing
  the error removes only 0.01 per step, so after 0.6 s the integral is still ≈ 9.4 and the
  output stays clamped at +0.5. The test fails.
- **With anti-windup**, the integral stops at ≈ 0.5 (where the output reaches the limit). After
  reversing, it crosses zero after ≈ 50 steps, so at 60 steps the output is negative. The test
  passes.

Floating-point values are compared with a tolerance (`ASSERT_NEAR`), never with `==`, because
of rounding error. On failure, each assertion prints the file, line, actual value, and expected
value. `main` returns non-zero if any check fails, so `make` reports the failure.

#### 5.3 Running the tests

```bash
make test     # run from the repository root
make clean    # delete build outputs
```

The `Makefile` compiles `tests/test_pid.c` together with `firmware/src/control/pid.c` using the
Mac's native compiler (Apple clang), with warnings enabled:

```
-std=c11 -Wall -Wextra -O0 -g -Ifirmware/include -Ifirmware/src/control
```

Recipes are written on the same line as the rule, separated by `;`
(`target: deps ; command`). This avoids the classic "missing separator" error that happens when
a copy-paste turns the required leading tab into spaces.

Build outputs go to `build/`, which is ignored by Git.

Final result:

```
All PID tests passed
```

---

### 6. Problems encountered and fixes

| Problem | Cause | Fix / lesson |
|---|---|---|
| `make: No rule to make target 'test'` | Ran `make` inside `firmware/`; the `Makefile` is in the repo root | Check the prompt / `pwd` before running `make`. Git commands work from any subfolder; `make` does not. |
| Pasted commands into the terminal had no effect | Pasted into the "Compile Project – Task" terminal, which only displays build output and is not a shell | Use a `zsh` terminal for commands; the Compile button is only for building. |
| Files were created as `PID.c` / `PID.h` while CMake referenced `pid.c` | macOS's file system is case-insensitive, so it still built locally; Linux (and CI) would fail | Renamed with `git mv` via a temporary name (a direct case-only rename may be ignored on macOS). Use lowercase file names for all C sources. |
| `p.kp`, `p.prev_err` compile errors | `p` is a pointer to a struct | Use `->` for pointers (`p->kp`), `.` only for a struct value. |
| `Kp` not found | C is case-sensitive; the field is `kp` | Match the exact field names in `pid.h`. |
| `float err;` used without a value | Declared but not initialized; C does not zero local variables | Always initialize: `float err = setpoint - measured;` |
| `elif` syntax error | Python habit | C uses `else if`, `&&`, `\|\|`, `!`. |
| Integral formula `(integ + prev_err) * dt` | Used the previous error and multiplied the old integral by `dt` | Correct form: `integ + err * dt` (only the new slice is multiplied by `dt`). |
| Clamping disappeared while adding anti-windup; an unconditional `integ = new_integ` remained | Added new logic on top of old logic instead of replacing it | The clamping test failed immediately — a regression caught by tests. When adding logic, check what it replaces. |

---

### 7. Known limitations

- **Derivative kick:** `prev_err` starts at 0, so if the first error is large, the derivative
  term spikes on the first update (e.g. 1 / 0.01 = 100). This is harmless while Kd = 0, which is
  the plan for the wheel-speed loop (low-resolution encoders make the derivative noisy). If the
  D term is used later, a common fix is to differentiate the measurement instead of the error.
- **Geometry constants are uncalibrated.** Wheel radius and track width in `config.h` come from
  the 2026-11-05 tape/wire measurements; they will be corrected by the 1 m straight-line and
  10-turn rotation tests in week 3.
- **The PID is not yet running on the robot.** It is tested on the host only; it will be wired
  into the 100 Hz control loop after the motor and encoder drivers exist.

---

### 8. Commits

| Commit | Message |
|---|---|
| `d2451fb` | Add include paths for control and estimation modules |
| `772104f` | Add config.h with geometry, control and pin constants |
| `c48fae6` | Add PID module interface and stub implementation |
| `0a40d8f` | Add PID unit tests and host Makefile |
| `3563dee` | Implement PID with output clamping and conditional-integration anti-windup |

---

### 9. Next steps

1. `odometry.c` + `tests/test_odometry.c` using the same test-first approach
   (straight line, in-place rotation, angle normalization).
2. When the multimeter arrives: measure motor resistance and battery voltage, then power wiring
   (LM2596 5 V rail) and voltage checks.
3. Motor driver (`motor.c`, PWM on GP2–GP5).
4. When the LM393 slot sensors arrive: encoder bring-up and confirm 20 pulses per wheel revolution.
