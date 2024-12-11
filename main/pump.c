#include "pump.h"
#include "driver/gpio.h"

#define PUMP_PIN (23)

void gpio_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PUMP_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en   = 0;
    gpio_config(&io_conf);
}

void pump_init(void)
{
    gpio_init();
}

void pump_on(void)
{
    gpio_set_level(PUMP_PIN, 1);
}

void pump_off(void)
{
    gpio_set_level(PUMP_PIN, 0);
}
