#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "alarm.h"

static const char* TAG = "Main";

static const int TASK_STACK_DEPTH = 16384;
static const int SM_TASK_PRIORITY = 1;
static const int TICK_PERIOD_MS = 250 / portTICK_PERIOD_MS;

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

void sm_task_handler(void* arg)
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

    sm.state = SETUP;

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
