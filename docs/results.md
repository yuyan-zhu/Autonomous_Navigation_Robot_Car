# Test Results

Every number reported in the README or on my resume comes from a test recorded here.
Each entry lists the date, the code version (Git commit), the test conditions, and the raw data.

---

## Control loop period jitter

**Goal:** check that the 100 Hz control loop runs at a steady 10 ms period, because the PID and
odometry math assume a fixed `dt`.

### 2026-11-07 — No load

- **Commit:** `c64b9ec`
- **Hardware:** Raspberry Pi Pico 2 (RP2350), USB-powered, no motors or sensors connected
- **Firmware:** `add_repeating_timer_us(-CONTROL_PERIOD_US, ...)` sets a flag in the timer
  callback; the main loop waits for the flag and timestamps each tick with `time_us_64()`
- **Load in the loop:** LED toggle every 100 ticks only (no PID or odometry yet)
- **Method:** measure the interval between consecutive ticks; report mean, min and max over
  10,000 periods (100 s), then reset and repeat
- **Output:** USB serial (`screen`), one line printed per 100 s window

**Raw output:**

```
Control loop jitter test, period = 10000 us
mean=10000.00 us  min=9996 us  max=10000 us
mean=10000.00 us  min=9999 us  max=10001 us
mean=10000.00 us  min=10000 us  max=10000 us
mean=10000.00 us  min=10000 us  max=10000 us
mean=10000.00 us  min=10000 us  max=10000 us
mean=10000.00 us  min=10000 us  max=10000 us
mean=10000.00 us  min=10000 us  max=10000 us
mean=10000.00 us  min=10000 us  max=10000 us
```

**Summary:**

| Window | Mean (µs) | Min (µs) | Max (µs) | Max deviation (µs) |
|---|---|---|---|---|
| 1 (startup) | 10000.00 | 9996 | 10000 | 4 |
| 2 | 10000.00 | 9999 | 10001 | 1 |
| 3–8 (steady state) | 10000.00 | 10000 | 10000 | < 1 |

**Analysis:**

- The mean period is exactly 10,000 µs in every window: the hardware timer has no long-term drift.
- In steady state (windows 3–8, about 10 minutes), every one of 60,000 periods measured exactly
  10,000 µs. Since `time_us_64()` has 1 µs resolution, the jitter is below the measurement
  resolution (< 1 µs).
- The deviations in the first two windows happened while the USB connection was being set up.
  The most likely cause is that the USB (TinyUSB) interrupt delayed the main loop by a few
  microseconds before it read the timestamp. A ±1 µs pair can also come from rounding to whole
  microseconds.
- The worst case (4 µs) is 0.04% of the 10 ms period, which is negligible for the PID and
  odometry calculations.

**Next:** repeat the test with simulated control work in the loop (two `pid_update` calls and
one `odom_update` call) to measure jitter under load.
