#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "config.h"
#include "pid.h"
#include "odometry.h"

#define JITTER_SAMPLES 10000   // 10000 ticks = 100 s at 100 Hz

static volatile bool g_tick = false;

// Hardware timer callback, runs every CONTROL_PERIOD_US.
// Keep it minimal: only set a flag. No printf, no float math.
static bool on_control_tick(repeating_timer_t *t) {
    (void)t;
    g_tick = true;
    return true;  // keep repeating
}

int main(void) {
    stdio_init_all();

    // Debug only: wait for the serial monitor. Remove when running untethered.
    while (!stdio_usb_connected()) {  // give the USB serial port time to connect
        sleep_ms(100);
    }  // Debug only!

    printf("Control loop jitter test, period = %d us\n", CONTROL_PERIOD_US);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    repeating_timer_t timer;
    add_repeating_timer_us(-CONTROL_PERIOD_US, on_control_tick, NULL, &timer);

    // LED heartbeat: counts control ticks
    uint32_t tick_count = 0;

    // Jitter statistics over JITTER_SAMPLES control periods
    uint64_t last_us = 0;
    uint32_t min_us = UINT32_MAX;
    uint32_t max_us = 0;
    uint64_t sum_us = 0;
    uint32_t count = 0;

    while (true) {
        while (!g_tick) {
            tight_loop_contents();
        }
        g_tick = false;

        // Timestamp first, as close to the tick as possible
        uint64_t now = time_us_64();

        // Toggle the LED every 100 ticks (once per second)
        tick_count = tick_count + 1;
        if (tick_count % 100 == 0) {
            gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        }

        // Measure the period since the last tick
        if (last_us != 0) {
            uint32_t period = (uint32_t)(now - last_us);
            if (period < min_us) {
                min_us = period;
            }
            if (period > max_us) {
                max_us = period;
            }
            sum_us += period;
            count++;
        }
        last_us = now;

        // Report and reset every JITTER_SAMPLES periods
        if (count == JITTER_SAMPLES) {
            printf("mean=%.2f us  min=%u us  max=%u us\n", 
                    (float)sum_us / count, (unsigned)min_us, (unsigned)max_us);
            min_us = UINT32_MAX;
            max_us = 0;
            sum_us = 0;
            count = 0;
        }
        







        // TODO (D): simulated control work
    }
}