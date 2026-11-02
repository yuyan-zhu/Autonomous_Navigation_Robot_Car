# Autonomous Navigation Robot Car

An autonomous navigation robot car built from scratch on the Raspberry Pi Pico 2
in pure C (Pico SDK): PID wheel-speed control, differential-drive odometry,
occupancy-grid mapping, and A* path planning.

## Status
- [x] Week 1: dev environment, project skeleton
- [ ] Week 1: power, motor driver, encoders, telemetry
- [ ] Week 2: closed-loop PID speed control

## Build
Open `firmware/` in VS Code with the Raspberry Pi Pico extension, then click **Compile** and **Run**.

## Repository layout
- `firmware/` – C code running on the Pico
- `tools/` – Python tools on the PC (plotting, simulator)
- `tests/` – unit tests compiled on the PC
- `docs/` – wiring, test results, dev log