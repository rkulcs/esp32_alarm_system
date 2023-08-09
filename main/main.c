#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "alarm.h"

static const char* TAG = "Main";

static const int TASK_STACK_DEPTH = 16384;
static const int SM_TASK_PRIORITY = 1;
static const int TICK_PERIOD_MS = 250 / portTICK_PERIOD_MS;

static const int PIN_BUTTON = 5;

typedef enum
{
    SETUP,
    SENSING,
    ALARM,
    DISARMED
} State;

typedef struct
{
    State state;
} StateMachine;

StateMachine sm;

static void IRAM_ATTR button_interrupt_handler(void* args)
{
    if (sm.state == ALARM)
        sm.state = DISARMED;
}

void sm_task_handler(void* args)
{
    while (true)
    {
        switch (sm.state)
        {
        case SETUP:
            break;
        case SENSING:
            break;
        case ALARM:
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

void init()
{
    set_up_alarm();

    gpio_reset_pin(PIN_BUTTON);
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_dis(PIN_BUTTON);
    gpio_pulldown_en(PIN_BUTTON);
    gpio_set_intr_type(PIN_BUTTON, GPIO_INTR_LOW_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_BUTTON, button_interrupt_handler, NULL);

    sm.state = ALARM;

    xTaskCreate(&sm_task_handler, "StateMachine", TASK_STACK_DEPTH, NULL, 
        SM_TASK_PRIORITY, NULL);
}

void app_main(void)
{
    init();

    while (1) 
    {
        vTaskDelay(100);
    }
}
