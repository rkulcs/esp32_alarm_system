#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "alarm.h"
#include "sensor.h"
#include "lcd1602.h"
#include "network.h"

static const int TICK_PERIOD_MS = 250 / portTICK_PERIOD_MS;
static const int DISARMED_PERIOD_MS = 3000 / portTICK_PERIOD_MS;

static const int PIN_BUTTON = 18;

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
    set_up_wifi();

    i2c_init();
    lcd_init();
    lcd_clear();

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
            lcd_write_one_line("Sensing...");
            break;
        case SENSING:
            if (sensor_detect_intrusion())
                state = ALARMED;
            break;
        case ALARMED:
            alert_user();
            break;
        case DISARMED:
            lcd_write_one_line("Disarmed.");
            vTaskDelay(DISARMED_PERIOD_MS);
            state = SETUP;
            break;
        default:
            break;
        }

        vTaskDelay(TICK_PERIOD_MS);
    }
}
