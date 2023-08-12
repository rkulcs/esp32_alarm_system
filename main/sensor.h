#pragma once

#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "driver/gpio.h"
#include "driver/mcpwm_cap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "circular_buffer.h"

void set_up_sensor();
void sensor_set_safe_distances();
double sensor_get_distance_in_cm();
