#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "alarm.h"
#include "sensor.h"

static const int TICK_PERIOD_MS = 250 / portTICK_PERIOD_MS;

static const int PIN_BUTTON = 5;

typedef enum
{
    SETUP,
    SENSING,
    ALARMED,
    DISARMED
} State;

State state;

static void IRAM_ATTR button_interrupt_handler(void* args)
{
    if (state == ALARMED)
        state = DISARMED;
}

void init()
{
    set_up_alarm();
    set_up_sensor();
    init_circular_buffer(&distance_buffer);

    gpio_reset_pin(PIN_BUTTON);
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_dis(PIN_BUTTON);
    gpio_pulldown_en(PIN_BUTTON);
    gpio_set_intr_type(PIN_BUTTON, GPIO_INTR_LOW_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_BUTTON, button_interrupt_handler, NULL);

    state = SETUP;
}

void app_main(void)
{
    init();

    while (true)
    {
        switch (state)
        {
        case SETUP:
            sensor_set_safe_distances();
            state = SENSING;
            break;
        case SENSING:
            break;
        case ALARMED:
            alert_user();
            break;
        case DISARMED:
            break;
        default:
            break;
        }

        vTaskDelay(TICK_PERIOD_MS);
    }
}
