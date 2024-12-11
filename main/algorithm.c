#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pump.h"
#include "water_detector.h"

static const char *TAG = "algorithm";

#define WATER_LEVEL_PUMP_ON  1270
#define WATER_LEVEL_PUMP_OFF 1490
#define TIME_SLOT_MS         10000

bool is_pump_on = false;

static void task(void *pvParameter)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(TIME_SLOT_MS));
        int water_level = water_detector_get_level();
        if (water_level < WATER_LEVEL_PUMP_ON && (!is_pump_on))
        {
            ESP_LOGI(TAG, "pump on");
            pump_on();
            is_pump_on = true;
        }
        else if (water_level > WATER_LEVEL_PUMP_OFF && is_pump_on)
        {
            vTaskDelay(pdMS_TO_TICKS(3000));
            ESP_LOGI(TAG, "pump off");
            pump_off();
            is_pump_on = false;
        }
    }
}

void algorithm_init(void)
{
    xTaskCreate(&task, "algorithm", 4096, NULL, 5, NULL);
}