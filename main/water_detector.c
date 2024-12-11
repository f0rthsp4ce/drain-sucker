#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"

static const char *TAG = "Touch pad";
static int water_level = 0;

#define TOUCH_THRESH_NO_USE          (0)
#define TOUCH_THRESH_PERCENT         (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (100)

#define TOUCH_PAD (0)

/*
T0	GPIO4
T1	GPIO0
T2	GPIO2
T3	MTDO
T4	MTCK
T5	MTDI
T6	MTMS
T7	GPIO27
T8	32K_XN
T9	32K_XP

    TOUCH_PAD_NUM0,      GPIO4(ESP32)
    TOUCH_PAD_NUM1,      GPIO0(ESP32)
    TOUCH_PAD_NUM2,      GPIO2(ESP32)
    TOUCH_PAD_NUM3,      GPIO15(ESP32)
    TOUCH_PAD_NUM4,      GPIO13(ESP32)
    TOUCH_PAD_NUM5,      GPIO12(ESP32)
    TOUCH_PAD_NUM6,      GPIO14(ESP32)
    TOUCH_PAD_NUM7,      GPIO27(ESP32)
    TOUCH_PAD_NUM8,      GPIO33(ESP32)
    TOUCH_PAD_NUM9,      GPIO32(ESP32)
*/

static touch_pad_t touch_pads_table[] = {
    TOUCH_PAD_NUM0,
};

#define TOUCH_PAD_TABLE_SIZE (sizeof(touch_pads_table)) / sizeof(touch_pads_table[0])

static bool s_pad_activated[TOUCH_PAD_TABLE_SIZE];
static uint32_t s_pad_init_val[TOUCH_PAD_TABLE_SIZE];

/*
  Read values sensed at all available touch pads.
  Use 2 / 3 of read value as the threshold
  to trigger interrupt when the pad is touched.
  Note: this routine demonstrates a simple way
  to configure activation threshold for the touch pads.
  Do not touch any pads when this routine
  is running (on application start).
 */
static void tp_detect_default_values(void)
{
    uint16_t touch_value;
    for (int i = 0; i < TOUCH_PAD_TABLE_SIZE; i++)
    {
        // read filtered value
        touch_pad_read_filtered(touch_pads_table[i], &touch_value);
        s_pad_init_val[i] = touch_value;
        ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", i, touch_value);
        // set interrupt threshold.
    }
}

int water_detector_get_level(void)
{
    return water_level;
}

/*
this task polling value from touch sesnor
 */
static void task(void *pvParameter)
{
    // tp_detect_default_values();
    static int show_message = 0;
    while (1)
    {
        uint16_t value = 0;
        for (int i = 0; i < TOUCH_PAD_TABLE_SIZE; i++)
        {
            touch_pad_read_filtered(touch_pads_table[i], &value);
            water_level = value;
            printf("T%d value: %4" PRIu16 "; ", touch_pads_table[i], value);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        printf("\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    // clear interrupt
    touch_pad_clear_status();
    for (int i = 0; i < TOUCH_PAD_TABLE_SIZE; i++)
    {
        if ((pad_intr >> touch_pads_table[i]) & 0x01)
        {
            s_pad_activated[i] = true;
        }
    }
}

/*
 * Before reading touch pad, we need to initialize the RTC IO.
 */
static void tp_example_touch_pad_init(void)
{
    for (int i = 0; i < TOUCH_PAD_TABLE_SIZE; i++)
    {
        // init RTC IO and mode for touch pad.
        touch_pad_config(touch_pads_table[i], TOUCH_THRESH_NO_USE);
    }
}

void water_detector_init(void)
{
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing touch pad");
    ESP_ERROR_CHECK(touch_pad_init());
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_SW);

    // Set reference voltage for charging/discharging
    // For most usage scenarios, we recommend using the following combination:
    // the high reference valtage will be 2.7V - 1V = 1.7V, The low reference voltage will be 0.5V.
    touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_HVOLT_ATTEN_1V);
    // Init touch pad IO
    tp_example_touch_pad_init();
    // Initialize and start a software filter to detect slight change of capacitance.
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // Set thresh hold

    // Start a task to show what pads have been touched
    xTaskCreate(&task, "touch_pad_read_task", 4096, NULL, 5, NULL);
}