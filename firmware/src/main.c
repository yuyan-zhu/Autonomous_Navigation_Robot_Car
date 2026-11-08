#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "config.h"
#include "pid.h"
#include "odometry.h"

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

    // TODO (B): tick counter for the LED
    // TODO (C): jitter statistics variables

    while (true) {
        while (!g_tick) {
            tight_loop_contents();
        }
        g_tick = false;

        // TODO (B): toggle the LED every 100 ticks
        // TODO (C): measure the period since the last tick
        // TODO (D): simulated control work
    }
}