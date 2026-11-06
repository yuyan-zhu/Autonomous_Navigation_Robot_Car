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
