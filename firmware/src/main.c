#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();                         // 初始化 USB 串口，printf 才能输出到电脑

    gpio_init(PICO_DEFAULT_LED_PIN);          // 初始化板载 LED 引脚（Pico 2 上是 GP25）
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    int i = 0;
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);    // 亮
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);    // 灭
        sleep_ms(500);
        printf("hello %d\n", i++);            // 每秒打印一次
    }
}




