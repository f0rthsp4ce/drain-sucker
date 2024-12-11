/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_log.h"
#include <inttypes.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"

#include "algorithm.h"
#include "pump.h"
#include "water_detector.h"

void app_main(void)
{
    pump_init();
    water_detector_init();
    algorithm_init();
}
