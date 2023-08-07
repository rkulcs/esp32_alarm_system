#include "alarm.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const int PIN_LED = 2;
static const int PIN_BUZZER = 15;

static const TickType_t ALARM_PERIOD = 100 / portTICK_PERIOD_MS;

void set_up_alarm()
{
    gpio_reset_pin(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_BUZZER);
    gpio_set_direction(PIN_BUZZER, GPIO_MODE_OUTPUT);
}

void alert_user()
{
    gpio_set_level(PIN_LED, 1);
    gpio_set_level(PIN_BUZZER, 1);
    vTaskDelay(ALARM_PERIOD);
    gpio_set_level(PIN_LED, 0);
    gpio_set_level(PIN_BUZZER, 0);
}
